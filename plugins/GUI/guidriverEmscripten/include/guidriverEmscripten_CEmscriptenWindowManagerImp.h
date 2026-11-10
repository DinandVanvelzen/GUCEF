/*
 *  guidriverEmscripten: GUCEF window driver for web browsers: a canvas with WebGL 2
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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWMANAGERIMP_H
#define GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWMANAGERIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#include "guidriverEmscripten_macros.h"
#define GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Window manager backend "Emscripten": the canvas of a web page with a WebGL 2 context, see CEmscriptenWindowContext */
class GUIDRIVEREMSCRIPTEN_PUBLIC_CPP CEmscriptenWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:

    CEmscriptenWindowManagerImp( void );

    virtual ~CEmscriptenWindowManagerImp();

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

    /** Resizes the canvas and enters or leaves full screen; the frequency is that of the display */
    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEmscriptenWindowManagerImp( const CEmscriptenWindowManagerImp& src );
    CEmscriptenWindowManagerImp& operator=( const CEmscriptenWindowManagerImp& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CEmscriptenWindowManagerImp, MT::CMutex > TEmscriptenWindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWMANAGERIMP_H ? */
