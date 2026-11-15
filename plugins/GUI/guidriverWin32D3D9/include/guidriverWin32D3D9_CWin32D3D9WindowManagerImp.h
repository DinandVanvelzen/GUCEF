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
 
#ifndef GUIDRIVERWIN32D3D9_CWIN32GLWINDOWMANAGERIMP_H
#define GUIDRIVERWIN32D3D9_CWIN32GLWINDOWMANAGERIMP_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

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

class GUIDRIVERWIN32D3D9_PUBLIC_CPP CWin32D3D9WindowManagerImp : public GUI::CWindowManagerBackend
{
    public:
    
    CWin32D3D9WindowManagerImp( void );
    
    virtual ~CWin32D3D9WindowManagerImp();
    
    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     );

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                        const GUI::CVideoSettings& settings   ,
                                                        const CORE::CValueList* params = GUCEF_NULL );

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params );
    
    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext );
    
    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   );

    /** None of the values: the windows have a Direct3D 9 device, which the GraphicsApi values do not name */
    virtual bool IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWin32D3D9WindowManagerImp( const CWin32D3D9WindowManagerImp& src );
    CWin32D3D9WindowManagerImp& operator=( const CWin32D3D9WindowManagerImp& src );        
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWin32D3D9WindowManagerImp, MT::CMutex > TWin32D3D9WindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32D3D9 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32D3D9_CWIN32GLWINDOWMANAGERIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
