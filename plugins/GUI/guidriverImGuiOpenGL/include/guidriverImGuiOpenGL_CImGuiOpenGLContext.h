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

#ifndef GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H
#define GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H
#include "guidriverImGuiOpenGL_CImGuiGLFunctions.h"
#define GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H
#endif /* GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H ? */

#ifndef GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H
#include "guidriverImGuiOpenGL_CImGuiGLFramebuffer.h"
#define GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H
#endif /* GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H ? */

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
 *  A Dear ImGui context drawn with OpenGL (OpenGL ES) into the GL window it was made for, with the OpenGL 3 backend of
 *  Dear ImGui (imgui_impl_opengl3).
 *
 *  Drawing: the draw data of a frame is drawn straight into the framebuffer of the window (GL_FRAMEBUFFER of the
 *  window), with the mapping of the present rectangle (CImGuiContext::GetPresentMapping()): the logical picture lands
 *  in the present rectangle at the present scale and Dear ImGui bakes its fonts at that scale, so there is no picture
 *  of its own to scale. The bottom layer clears the window to opaque black first, higher layers are blended over it.
 *  The backend serves the texture requests of the draw data (the font atlas, the images and bitmap font atlases of the
 *  texture cache).
 *
 *  Captures: CaptureFrame() draws the last draw data again, without a new frame, into an offscreen framebuffer of the
 *  render size (CImGuiGLFramebuffer), on opaque black, and reads it back. CapturePresentedFrame() reads the window.
 *
 *  Everything happens with the GL context of the window current, on the thread of the window.
 */
class GUIDRIVERIMGUIGL_PUBLIC_CPP CImGuiOpenGLContext : public GUIDRIVERIMGUI::CImGuiContext
{
    public:

    /**
     *  Makes the context and sets the backend up with the GLSL version string given ("#version 330 core", "#version 300 es"):
     *  see SelectGlslVersion(). The GL context of the window must be current. IsRendererReady() tells whether it worked
     */
    CImGuiOpenGLContext( GUIDRIVERIMGUI::CImGuiDriver* driver         ,
                         GUI::TWindowContextPtr windowContext         ,
                         const CORE::CString& glslVersion             ,
                         const CImGuiGLFunctions& functions           ,
                         CImGuiGLFunctions::TGetProcAddress getProcAddress );

    /** Lets go of the backend and every GL object, with the GL context of the window current */
    virtual ~CImGuiOpenGLContext();

    /** The backend is set up, the shaders compiled: the context can draw */
    bool IsRendererReady( void ) const;

    /** The GLSL version string the backend was given */
    const CORE::CString& GetGlslVersion( void ) const;

    /** An OpenGL texture of the application as a texture reference for ImGui::Image(): the name is the ImTextureID */
    ImTextureRef GetTextureRef( UInt32 glTexture ) const;

    /** How many frames this context drew into its window */
    UInt32 GetDrawnFrameCount( void ) const;

    /** The window as it is now (what the last frame left in its framebuffer), RGBA with the rows from the top */
    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The GLSL version string for a window, from its GL_API ("GL" or "GLES") and GL_VERSION ("3.3") properties:
     *  - OpenGL 3.3 or later: "#version 330 core"; a window with an older or a compatibility context (GL_VERSION without
     *    a number, "1.x") is refused;
     *  - OpenGL ES 3.0 or later (WebGL 2): "#version 300 es";
     *  - a build for OpenGL ES (Android, the web) refuses a desktop OpenGL window.
     *  Returns false, with the reason in 'reason', when the window cannot be drawn in
     */
    static bool SelectGlslVersion( const CORE::CString& api   ,
                                   const CORE::CString& version ,
                                   bool openGlEsBuild         ,
                                   CORE::CString& glslVersion ,
                                   CORE::CString& reason      );

    protected:

    virtual bool OnRendererNewFrame( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RenderDrawData( ImDrawData& drawData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReleaseRenderer( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CImGuiOpenGLContext( const CImGuiOpenGLContext& src );
    CImGuiOpenGLContext& operator=( const CImGuiOpenGLContext& src );

    UInt32 GetWindowFramebuffer( void ) const;

    private:

    CImGuiGLFunctions m_gl;
    CImGuiGLFunctions::TGetProcAddress m_getProcAddress;
    CORE::CString m_glslVersion;
    CImGuiGLFramebuffer m_framebuffer;
    UInt32 m_drawnFrames;
    bool m_backendReady;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H ? */
