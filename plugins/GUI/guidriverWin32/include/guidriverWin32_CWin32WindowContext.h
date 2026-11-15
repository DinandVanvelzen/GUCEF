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

#ifndef GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H
#define GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#include "gucefCORE_CMsWin32Window.h"
#define GUCEF_CORE_CMSWIN32WINDOW_H
#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVERWIN32_MACROS_H
#include "GUIDRIVERWIN32_macros.h"
#define GUIDRIVERWIN32_MACROS_H
#endif /* GUIDRIVERWIN32_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A plain Win32 window without a graphics context (GraphicsApi None, the only kind this backend makes): a renderer
 *  (Ogre with Direct3D 11 or OpenGL through WGL) makes its swap chain or context on the HWND.
 *
 *  Frames are announced (WindowContextRedrawEvent) on the pulses of the application at the frequency of the video
 *  settings and on WM_PAINT; the window neither clears nor presents. Full screen is a borderless window over the whole
 *  monitor (CMsWin32Window::SetFullscreenStatus), no change of the display mode. Closing the window (WM_CLOSE) notifies
 *  WindowContextCloseRequestEvent; CMsWin32Window destroys the window right after it, so a renderer lets go of its
 *  surface in that event.
 *
 *  Sizes are in physical pixels: unless the window parameter "DPI_AWARE" is "false" the process is made per monitor DPI
 *  aware (version 2) before its first window, where Windows offers it (Windows 10 1703 and newer), so Windows does not
 *  scale the picture of the window. The property SCALE is the DPI of the monitor of the window divided by 96, DPI the DPI.
 *
 *  Properties: WINDOW and HWND (the HWND), GRAPHICS_API ("None"), SCALE, DPI.
 */
class GUIDRIVERWIN32_PUBLIC_CPP CWin32WindowContext : public GUI::CWindowContext
{        
    public:
    
    CWin32WindowContext( void );
    
    virtual ~CWin32WindowContext();
    
    virtual void SetGuiContext( GUI::TGuiContextPtr& context );
    
    virtual GUI::TGuiContextPtr GetGuiContext( void );

    virtual GUI::UInt32 GetWidth( void ) const;

    virtual GUI::UInt32 GetHeight( void ) const;
    
    virtual GUI::UInt32 GetID( void ) const;
    
    virtual bool IsActive( void ) const;
    
    virtual GUI::CString GetName( void ) const;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const;

    /** A system cursor (LoadCursor) set as the cursor of the window class, so all windows of the class show it */
    virtual bool SetSystemCursorShape( GUI::TCursorShape shape );

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings );

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings ,
                     const CORE::CValueList* params           );

    /** Full screen or windowed; a windowed window takes the size of the settings; the pulses follow the frequency */
    bool ApplyVideoSettings( const GUI::CVideoSettings& videoSettings );

    /** The DPI of the monitor the window is on, 96 where Windows cannot tell (before Windows 10 1607) */
    GUI::UInt32 GetDpi( void ) const;

    void Shutdown( void );

    virtual const CORE::CString& GetClassTypeName( void ) const;

    protected:
   
    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventID        ,
                           CORE::CICloneable* evenData = GUCEF_NULL );
    
    private:
    
    CWin32WindowContext( const CWin32WindowContext& src );            /**< private because: must be unique */ 
    CWin32WindowContext& operator=( const CWin32WindowContext& src ); /**< private because: must be unique */

    bool MakeCurrent( void );

    void SetPulseFrequency( GUI::UInt32 frequency );

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    CORE::CMsWin32Window m_window;
    GUI::UInt32 m_pulseFrequency;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWin32WindowContext, MT::CMutex >  TWin32WindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2005 :
        - Initial version

---------------------------------------------------------------------------*/
