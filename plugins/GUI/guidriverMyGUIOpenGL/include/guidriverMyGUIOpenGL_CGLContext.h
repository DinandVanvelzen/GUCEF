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

#ifndef GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H
#define GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#include "guidriverMyGUI_CMyGUIContext.h"
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */

#ifndef GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H
#include "guidriverMyGUIOpenGL_IGLBackend.h"
#define GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H
#endif /* GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H ? */

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
 *  A MyGUI context that draws with OpenGL. The picture (the logical resolution times the render scale) is drawn
 *  by MyGUI into a framebuffer of its own, which is then scaled to the window the way the present mode of the
 *  context says; it is also what CaptureFrame() reads back.
 */
class GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP CGLContext : public guidriverMyGUI::CMyGUIContext
{
    public:

    /** Takes over the backend; IsValid() tells whether MyGUI could start with it */
    CGLContext( GUI::CGUIDriver* driver              ,
                GUI::TWindowContextPtr windowContext ,
                IGLBackend* backend                  );

    virtual ~CGLContext();

    bool IsValid( void ) const;

    /** The platform of MyGUI that draws: "OpenGL3", "OpenGL" or "OpenGLES" */
    const char* GetBackendName( void ) const;

    virtual bool IsBgraTextureMemory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGLContext( const CGLContext& src );
    CGLContext& operator=( const CGLContext& src );

    UInt32 GetWindowFramebuffer( void ) const;

    bool PrepareFramebuffer( UInt32 width, UInt32 height );

    void ReleaseFramebuffer( void );

    bool RenderPicture( void );

    void Present( void );

    private:

    IGLBackend* m_backend;
    bool m_isValid;
    UInt32 m_framebuffer;
    UInt32 m_colourBuffer;
    UInt32 m_width;
    UInt32 m_height;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H ? */
