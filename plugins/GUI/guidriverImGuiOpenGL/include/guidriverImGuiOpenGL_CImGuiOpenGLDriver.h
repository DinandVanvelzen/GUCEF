/*
 *  guidriverImGuiOpenGL: gucefGUI driver ImGui: Dear ImGui drawn with OpenGL
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

#ifndef GUIDRIVERIMGUIGL_CIMGUIOPENGLDRIVER_H
#define GUIDRIVERIMGUIGL_CIMGUIOPENGLDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERIMGUI_CIMGUIDRIVER_H
#include "guidriverImGui_CImGuiDriver.h"
#define GUIDRIVERIMGUI_CIMGUIDRIVER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRIVER_H ? */

#ifndef GUIDRIVERIMGUIGL_MACROS_H
#include "guidriverImGuiOpenGL_macros.h"
#define GUIDRIVERIMGUIGL_MACROS_H
#endif /* GUIDRIVERIMGUIGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The GUI driver "ImGui": Dear ImGui drawn with OpenGL (OpenGL ES) into windows that have a GL context
 *  (the window drivers XWinGL, Wayland, EGL, AndroidGLES and Emscripten).
 *
 *  A window without the property GL_GETPROCADDRESS (a window of the window backend Ogre, a window made with the graphics
 *  API None) is refused with an error log entry. The GLSL version follows the window: OpenGL 3.3 core ("#version 330 core"),
 *  OpenGL ES 3 and WebGL 2 ("#version 300 es"); see CImGuiOpenGLContext::SelectGlslVersion().
 *
 *  The entry points of OpenGL are found through the function of the first window (GL_GETPROCADDRESS), also those of the
 *  backend of Dear ImGui. Property "Renderer": the vendor, renderer and version strings of OpenGL.
 */
class GUIDRIVERIMGUIGL_PUBLIC_CPP CImGuiOpenGLDriver : public GUIDRIVERIMGUI::CImGuiDriver
{
    public:

    CImGuiOpenGLDriver( void );

    virtual ~CImGuiOpenGLDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    /** "Renderer": the strings of the first GL context a context was made in. The rest is the one of CImGuiDriver */
    virtual CORE::CString GetDriverProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CImGuiOpenGLDriver( const CImGuiOpenGLDriver& src );
    CImGuiOpenGLDriver& operator=( const CImGuiOpenGLDriver& src );

    private:

    CORE::CString m_glDescription;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_CIMGUIOPENGLDRIVER_H ? */
