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

#ifndef GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H
#define GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

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

#ifndef GUIDRIVERWIN32D3D9_MACROS_H
#include "GUIDRIVERWIN32D3D9_macros.h"
#define GUIDRIVERWIN32D3D9_MACROS_H
#endif /* GUIDRIVERWIN32D3D9_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32D3D9 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUIDRIVERWIN32D3D9_PUBLIC_CPP CWin32D3D9WindowContext : public GUI::CWindowContext
{        
    public:
    
    CWin32D3D9WindowContext( void );
    
    virtual ~CWin32D3D9WindowContext();
    
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

    void Shutdown( void );

    virtual const CORE::CString& GetClassTypeName( void ) const;

    protected:
   
    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventID        ,
                           CORE::CICloneable* evenData = GUCEF_NULL );
    
    private:
    
    CWin32D3D9WindowContext( const CWin32D3D9WindowContext& src );            /**< private because: must be unique */ 
    CWin32D3D9WindowContext& operator=( const CWin32D3D9WindowContext& src ); /**< private because: must be unique */

    bool MakeCurrent( void );

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    CORE::CMsWin32Window m_window;
    LPDIRECT3D9 m_pD3D9;
    LPDIRECT3DDEVICE9 m_pD3DDevice;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWin32D3D9WindowContext, MT::CMutex >  TWin32GLWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32D3D9 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2005 :
        - Initial version

---------------------------------------------------------------------------*/
