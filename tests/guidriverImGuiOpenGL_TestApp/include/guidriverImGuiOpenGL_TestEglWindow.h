/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#ifndef GUIDRIVERIMGUIGL_TESTEGLWINDOW_H
#define GUIDRIVERIMGUIGL_TESTEGLWINDOW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "guidriverImGuiOpenGL_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An offscreen window of the test: an EGL context without a surface and a framebuffer object of the size of the window,
 *  as the framebuffer of the window (GL_FRAMEBUFFER). 'gles' asks for OpenGL ES 3, else it is OpenGL 3.3 core. Unlike the
 *  window driver guidriverEGL it can be made as OpenGL ES on a machine that has both, and it can be resized.
 *  The context of the window is current after Initialize(); with several windows MakeCurrent() before using one.
 */
class CTestEglWindow : public GUI::CWindowContext
{
    public:

    explicit CTestEglWindow( bool gles );

    virtual ~CTestEglWindow();

    bool Initialize( UInt32 width, UInt32 height );

    void Shutdown( void );

    bool MakeCurrent( void );

    /** A new framebuffer of another size; the contexts of the window are told */
    bool Resize( UInt32 width, UInt32 height );

    /** The window as it is now, RGBA with the rows from the top */
    bool ReadPixels( IMAGE::TPixelMapPtr& pixels );

    /** The framebuffer object of the window */
    UInt32 GetFramebuffer( void ) const;

    /** What glGetError() says now: 0 is none; clears the error */
    UInt32 TakeGlError( void );

    const GUIDRIVERIMGUIGL::CImGuiGLFunctions& GetFunctions( void ) const;

    virtual UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTestEglWindow( const CTestEglWindow& src );
    CTestEglWindow& operator=( const CTestEglWindow& src );

    private:

    UInt32 m_id;
    bool m_gles;
    UInt32 m_width;
    UInt32 m_height;
    void* m_display;
    void* m_context;
    GUIDRIVERIMGUIGL::CImGuiGLFunctions m_gl;
    GUIDRIVERIMGUIGL::CImGuiGLFramebuffer* m_framebuffer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_TESTEGLWINDOW_H ? */
