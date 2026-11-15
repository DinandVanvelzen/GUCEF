/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
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

#ifndef GUCEF_guidriverMyGUIOpenGL_CGLDRIVER_H
#define GUCEF_guidriverMyGUIOpenGL_CGLDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUI_CMYGUIDRIVER_H
#include "guidriverMyGUI_CMyGUIDriver.h"
#define GUCEF_guidriverMyGUI_CMYGUIDRIVER_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIDRIVER_H ? */

#ifndef GUCEF_guidriverMyGUIOpenGL_MACROS_H
#include "guidriverMyGUIOpenGL_macros.h"
#define GUCEF_guidriverMyGUIOpenGL_MACROS_H
#endif /* GUCEF_guidriverMyGUIOpenGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The MyGUI GUI driver for windows with an OpenGL context. Which platform of MyGUI draws is decided when a
 *  context is made, from the OpenGL of the window: on desktops the OpenGL 3 platform (GLSL 330) from OpenGL 3.3
 *  on and the legacy OpenGL platform below that, on Android and in browsers the OpenGL ES 3 platform (WebGL 2).
 *  The window gives the OpenGL context through its properties GL_API, GL_VERSION and GL_FRAMEBUFFER.
 */
class GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP CGLDriver : public guidriverMyGUI::CMyGUIDriver
{
    public:

    static const char* DriverName;

    CGLDriver( void );

    virtual ~CGLDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGLDriver( const CGLDriver& src );
    CGLDriver& operator=( const CGLDriver& src );

    private:

    CString m_backendName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOpenGL_CGLDRIVER_H ? */
