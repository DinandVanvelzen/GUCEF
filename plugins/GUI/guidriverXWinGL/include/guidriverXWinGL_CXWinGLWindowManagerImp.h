/*
 *  guidriverXWinGL: module implementing GL based window management for X11
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

#ifndef GUIDRIVERXWINGL_CXWINGLWINDOWMANAGERIMP_H
#define GUIDRIVERXWINGL_CXWINGLWINDOWMANAGERIMP_H

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

#ifndef GUIDRIVERXWINGL_MACROS_H
#include "guidriverXWinGL_macros.h"
#define GUIDRIVERXWINGL_MACROS_H
#endif /* GUIDRIVERXWINGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERXWINGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUIDRIVERXWINGL_PUBLIC_CPP CXWinGLWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:

    CXWinGLWindowManagerImp( void );

    virtual ~CXWinGLWindowManagerImp();

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     );

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                        const GUI::CVideoSettings& settings   ,
                                                        const CORE::CValueList* params = NULL );

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params );

    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext );

    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   );

    private:

    CXWinGLWindowManagerImp( const CXWinGLWindowManagerImp& src );
    CXWinGLWindowManagerImp& operator=( const CXWinGLWindowManagerImp& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CXWinGLWindowManagerImp, MT::CMutex > TXWinGLWindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERXWINGL */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERXWINGL_CXWINGLWINDOWMANAGERIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
