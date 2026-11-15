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
 
#ifndef GUIDRIVERWIN32_CWIN32WINDOWMANAGERIMP_H
#define GUIDRIVERWIN32_CWIN32WINDOWMANAGERIMP_H 

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

class GUIDRIVERWIN32_PUBLIC_CPP CWin32WindowManagerImp : public GUI::CWindowManagerBackend
{
    public:
    
    CWin32WindowManagerImp( void );
    
    virtual ~CWin32WindowManagerImp();
    
    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                        const GUI::CVideoSettings& settings   ,
                                                        const CORE::CValueList* params = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   ) GUCEF_VIRTUAL_OVERRIDE;

    /** "None": the windows are plain windows without a graphics context */
    virtual bool IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private:

    CWin32WindowManagerImp( const CWin32WindowManagerImp& src );
    CWin32WindowManagerImp& operator=( const CWin32WindowManagerImp& src );        
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWin32WindowManagerImp, MT::CMutex > TWin32WindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32_CWIN32WINDOWMANAGERIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
