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

#ifndef GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H
#define GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

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

/**
 *  A window without a screen: an OpenGL context made with EGL (no display server needed, Mesa's
 *  software renderer works) that draws into a framebuffer object of the window's size.
 *
 *  It is used like any other window context: renderers bind the framebuffer named by the
 *  GL_FRAMEBUFFER property instead of framebuffer 0, and load GL functions with the function named
 *  by GL_GETPROCADDRESS. The context is current on the thread that created the window.
 *
 *  Properties: GL_FRAMEBUFFER, GL_GETPROCADDRESS, GL_API ("GL" or "GLES"), GL_VERSION ("3.3"),
 *  OFFSCREEN ("true").
 *
 *  With a frequency in the video settings the window asks for a redraw that often (driven by the
 *  application's pulse generator); RequestRedraw() draws a frame right away.
 */
class GUIDRIVEREGL_PUBLIC_CPP CEGLWindowContext : public GUI::CWindowContext
{
    public:

    CEGLWindowContext( void );

    virtual ~CEGLWindowContext();

    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings );

    void Shutdown( void );

    /** Changes the size of the picture */
    bool Resize( GUI::UInt32 width, GUI::UInt32 height );

    /** Makes the GL context current on the calling thread */
    bool MakeCurrent( void );

    /** Clears the picture and lets the observers draw a frame */
    void RequestRedraw( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool CreateFramebuffer( GUI::UInt32 width, GUI::UInt32 height );

    void DestroyFramebuffer( void );

    CEGLWindowContext( const CEGLWindowContext& src );
    CEGLWindowContext& operator=( const CEGLWindowContext& src );

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    GUI::UInt32 m_width;
    GUI::UInt32 m_height;
    void* m_display;            /**< EGLDisplay */
    void* m_context;            /**< EGLContext */
    void* m_surface;            /**< EGLSurface, a 1 x 1 pbuffer when the context cannot be current without one */
    bool m_isGles;
    GUI::UInt32 m_framebuffer;
    GUI::UInt32 m_colourBuffer;
    GUI::UInt32 m_depthStencilBuffer;
    bool m_drivesRedraws;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CEGLWindowContext, MT::CMutex > TEGLWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H ? */
