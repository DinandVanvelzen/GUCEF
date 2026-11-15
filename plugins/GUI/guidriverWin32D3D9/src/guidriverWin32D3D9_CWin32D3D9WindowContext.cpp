/*
 *  guidriverWin32D3D9: module implementing D3D9 based window management for Win32
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

#include "GUIDRIVERWIN32D3D9_CWin32D3D9WindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32D3D9 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != GUCEF_NULL)        \
   {                    \
      x->Release();     \
      x = GUCEF_NULL;         \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != GUCEF_NULL)             \
   {                         \
      delete[] x;            \
      x = GUCEF_NULL;              \
   }
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/


CWin32D3D9WindowContext::CWin32D3D9WindowContext( void )
    : CWindowContext()         ,
      m_guiContext()           ,
      m_id()                   ,
      m_name()                 ,
      m_window()               ,
      m_pD3D9( GUCEF_NULL )          ,
      m_pD3DDevice( GUCEF_NULL )
{GUCEF_TRACE;

    SubscribeTo( &m_window );
}

/*-------------------------------------------------------------------------*/

CWin32D3D9WindowContext::~CWin32D3D9WindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CWin32D3D9WindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CWin32D3D9WindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32D3D9WindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CWin32D3D9WindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWin32D3D9WindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWin32D3D9WindowContext::Shutdown( void )
{GUCEF_TRACE;

	CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestStopOfPeriodicUpdates( this );
    UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
    
    SAFE_RELEASE( m_pD3DDevice ); 
    SAFE_RELEASE( m_pD3D9 );

    if ( GUCEF_NULL != m_window.GetHwnd() )
    {
        CORE::CMsWin32Window::UnregisterWindowClass( m_window.GetText() );
        m_window.WindowDestroy();
    }
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32D3D9WindowContext::GetWidth( void ) const
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
CWin32D3D9WindowContext::GetHeight( void ) const
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
CWin32D3D9WindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( propertyName == "WINDOW" || propertyName == "HWND" )
    {
        return CORE::PointerToString( m_window.GetHwnd() );
    }
    return GUI::CString(); 
}

/*-------------------------------------------------------------------------*/

bool
CWin32D3D9WindowContext::Initialize( const GUI::CString& title                ,
                                     const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    // Do not initialize twice
    Shutdown();

    if ( !CORE::CMsWin32Window::RegisterWindowClass( GetClassTypeName() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Could not register window class" );
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
        // Create Direct3D9 Object 
        LPDIRECT3D9 pD3D9 = Direct3DCreate9( D3D_SDK_VERSION ); 
        if ( GUCEF_NULL == pD3D9 ) 
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Could not create D3D9 device object. SDK Version " + CORE::Int32ToString( D3D_SDK_VERSION ) );
            Shutdown();
		    return false;
        }
 
        // Fill out the presentation parameters 
        D3DPRESENT_PARAMETERS D3Dpp; 
        ZeroMemory( &D3Dpp, sizeof(D3Dpp) ); 
        D3Dpp.Windowed = TRUE; 
        D3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
 
        // Create the Direct3D Device 
        LPDIRECT3DDEVICE9 pD3DDevice = GUCEF_NULL; 
        if ( FAILED( pD3D9->CreateDevice( D3DADAPTER_DEFAULT, 
                                          D3DDEVTYPE_HAL, 
                                          m_window.GetHwnd(),
                                          D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
                                          &D3Dpp,
                                          &pD3DDevice ) ) ) 
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Could not create D3D9 device" );
            SAFE_RELEASE( pD3D9 ); 
            Shutdown();
		    return false;
        }	    

        m_pD3D9 = pD3D9;
        m_pD3DDevice = pD3DDevice;

        // Display the new window
        m_window.Show();
        m_window.SendToForegound();
        m_window.GrabFocus();

        // Grab the main app pulse generator and set the update interval for the context to the desired refresh rate
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator(); 
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways() );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Succesfully created Direct3D9 rendering context" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWin32D3D9WindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::GUIDRIVERWIN32D3D9::CWin32D3D9WindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWin32D3D9WindowContext::OnNotify( CORE::CNotifier* notifier   ,
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
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Window resize event received" );
        NotifyObservers( WindowContextSizeEvent );
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowActivationEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Window activation event received" );
        NotifyObservers( WindowContextActivateEvent );
    }
    else
    if ( ( eventID == CORE::CMsWin32Window::WindowCloseEvent )    ||
         ( eventID == CORE::CMsWin32Window::WindowDestroyEvent )  )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32D3D9WindowContext: Window close/destroy event received" );
        Shutdown();
    }
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32D3D9 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
