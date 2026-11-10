/*
 *  guidriverEGL: GUCEF window driver giving offscreen OpenGL windows through EGL, for tests, tools and headless runs
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_GUIDRIVEREGL_CEGLWINDOWMANAGERIMP_H
#define GUCEF_GUIDRIVEREGL_CEGLWINDOWMANAGERIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUIDRIVEREGL_MACROS_H
#include "guidriverEGL_macros.h"
#define GUCEF_GUIDRIVEREGL_MACROS_H
#endif /* GUCEF_GUIDRIVEREGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Window manager backend "EGL": offscreen OpenGL windows, see CEGLWindowContext */
class GUIDRIVEREGL_PUBLIC_CPP CEGLWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:

    CEGLWindowManagerImp( void );

    virtual ~CEGLWindowManagerImp();

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title                   ,
                                                        const GUI::CVideoSettings& settings         ,
                                                        const CORE::CValueList* params = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    /** Resizes the picture; the other settings mean nothing without a screen */
    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEGLWindowManagerImp( const CEGLWindowManagerImp& src );
    CEGLWindowManagerImp& operator=( const CEGLWindowManagerImp& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CEGLWindowManagerImp, MT::CMutex > TEGLWindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVEREGL_CEGLWINDOWMANAGERIMP_H ? */
