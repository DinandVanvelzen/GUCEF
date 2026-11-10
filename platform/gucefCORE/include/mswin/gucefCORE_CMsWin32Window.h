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

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#define GUCEF_CORE_CMSWIN32WINDOW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* module config macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_MSWIN_BUILD

#ifndef GUCEF_CORE_COSWINDOW_H
#include "gucefCORE_COSWindow.h"
#define GUCEF_CORE_COSWINDOW_H
#endif /* GUCEF_CORE_COSWINDOW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class with basic utilities for creating a Win32 window
 */
class GUCEF_CORE_PUBLIC_CPP CMsWin32Window : public COSWindow
{
    public:
    
    CMsWin32Window();

    virtual ~CMsWin32Window();
    
    void Close( void );
    
    virtual void SetText( const CString& text ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CString GetText( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    HWND GetHwnd( void ) const;
    
    HWND GetParentHwnd( void ) const;
    
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool WindowCreate( const CString& windowClassName ,
                               const CString& windowTitle     ,
                               const Int32 xPosition          ,
                               const Int32 yPosition          ,
                               const UInt32 width             ,
                               const UInt32 height            ,
                               const HWND hWndParent = NULL   );

    virtual bool WindowCreate( const CString& windowTitle     ,
                               const Int32 xPosition          ,
                               const Int32 yPosition          ,
                               const UInt32 width             ,
                               const UInt32 height            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void WindowDestroy( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    static bool RegisterWindowClass( const CString& windowClassName );

    static void UnregisterWindowClass( const CString& windowClassName );

    bool Show( void );
    
    bool Hide( void );
    
    bool Maximize( void );
    
    bool Minimize( void );
    
    void Repaint( void );

    void SendToForegound( void );

    void GrabFocus( void );

    /**
     *  Full screen: a window without borders that covers the monitor it is on. Going back restores the
     *  style and the place the window had before.
     */
    void SetFullscreenStatus( bool fullscreen );

    bool GetFullscreenStatus( void ) const;
    
    bool GetWindowAreaPosition( int& x, int& y ) const;
    
    bool GetClientAreaPosition( int& x, int& y ) const;
    
    bool GetWindowArea( int& x, int& y, int& width, int& height ) const;

    virtual int GetWidthOfClientAreaBorder( void ) const;
    
    bool GetWindowAreaRelativeToParentClientArea( Float32& x      , 
                                                  Float32& y      ,
                                                  Float32& width  , 
                                                  Float32& height ) const;
    
    bool GetClientArea( int& x, int& y, int& width, int& height ) const;
    
    void WindowAreaResize( int x, int y, int width, int height, bool repaint = true );
    
    void WindowAreaResize( int width, int height, bool repaint = true );
    
    void ClientAreaResize( int x, int y, int width, int height, bool repaint = true );
    
    void ClientAreaResize( int width, int height, bool repaint = true );

    void WindowAreaResizeRelativeToParentClientArea( Float32 x           , 
                                                     Float32 y           ,
                                                     Float32 width       , 
                                                     Float32 height      ,
                                                     bool repaint = true );
    
    void ClientAreaResizeRelativeToParentClientArea( Float32 x           ,
                                                     Float32 y           ,
                                                     Float32 width       , 
                                                     Float32 height      ,
                                                     bool repaint = true );

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual LRESULT WindowProc( const HWND hWnd     ,
                                const UINT nMsg     ,
                                const WPARAM wParam ,
                                const LPARAM lParam );

    void SetHwnd( HWND hwnd );
    
    void SetOriginalWinProc( WNDPROC originalWinProc );
    
    WNDPROC GetOriginalWinProc( void );
    
    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE; 

    static LRESULT WndProc( HWND hwnd     , 
                            UINT msg      ,
                            WPARAM wParam ,
                            LPARAM lParam );
                            
    void HookWndProc( void );

    private:

    CMsWin32Window( const CMsWin32Window& src );       /**< cannot be copied */
    CMsWin32Window& operator=( const CMsWin32Window ); /**< cannot be copied */

    private:
    
    HWND m_hwnd;
    WNDPROC m_orgWinProc;
    bool m_isFullscreen;
    LONG_PTR m_windowedStyle;   /**< the style before going full screen */
    RECT m_windowedRect;        /**< the window area before going full screen */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */

#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
