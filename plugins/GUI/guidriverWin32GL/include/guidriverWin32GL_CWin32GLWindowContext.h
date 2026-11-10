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

#ifndef GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H
#define GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Windows.h>

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#include "gucefCORE_CMsWin32Window.h"
#define GUCEF_CORE_CMSWIN32WINDOW_H
#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVERWIN32GL_MACROS_H
#include "guidriverWin32GL_macros.h"
#define GUIDRIVERWIN32GL_MACROS_H
#endif /* GUIDRIVERWIN32GL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32GL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUIDRIVERWIN32GL_PUBLIC_CPP CWin32GLWindowContext : public GUI::CWindowContext
{        
    public:
    
    CWin32GLWindowContext( void );
    
    virtual ~CWin32GLWindowContext();
    
    virtual void SetGuiContext( GUI::TGuiContextPtr& context );
    
    virtual GUI::TGuiContextPtr GetGuiContext( void );

    virtual GUI::UInt32 GetWidth( void ) const;

    virtual GUI::UInt32 GetHeight( void ) const;
    
    virtual GUI::UInt32 GetID( void ) const;
    
    virtual bool IsActive( void ) const;
    
    virtual GUI::CString GetName( void ) const;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const;

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings );

    /** Full screen or a window of the size of the settings, waiting for the vertical blank or not */
    bool ApplyVideoSettings( const GUI::CVideoSettings& videoSettings );

    void Shutdown( void );

    virtual const CORE::CString& GetClassTypeName( void ) const;

    protected:
   
    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventID        ,
                           CORE::CICloneable* evenData = NULL );
    
    private:
    
    CWin32GLWindowContext( const CWin32GLWindowContext& src );            /**< private because: must be unique */ 
    CWin32GLWindowContext& operator=( const CWin32GLWindowContext& src ); /**< private because: must be unique */

    bool MakeCurrent( void );

    /** Swaps wait for the vertical blank or not; false when the driver cannot set it */
    bool ApplySwapInterval( bool vsync );

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    CORE::CMsWin32Window m_window;
    HGLRC m_renderContext;
    HDC m_deviceContext;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWin32GLWindowContext, MT::CMutex >  TWin32GLWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32GL */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2005 :
        - Initial version

---------------------------------------------------------------------------*/
