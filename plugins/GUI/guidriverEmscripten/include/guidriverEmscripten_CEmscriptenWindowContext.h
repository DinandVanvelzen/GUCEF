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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H
#define GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H

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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#include "guidriverEmscripten_macros.h"
#define GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace CORE { class CValueList; } }

namespace GUCEF {
namespace GUIDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A window in a web page: a canvas element with a WebGL 2 context (OpenGL ES 3.0 for the renderers).
 *
 *  The canvas is named by the window parameter "CANVAS", a CSS selector ("#canvas" when not given, the canvas of
 *  the page Emscripten makes). The drawing buffer of the canvas gets the size of the video settings; the page
 *  decides with CSS how large it is shown. Full screen shows the canvas on the whole screen, scaled with its
 *  aspect ratio kept and nearest filtering; a browser only allows that in the handler of a user's input, a request
 *  made elsewhere is carried out at the next key press or click.
 *
 *  Properties: WINDOW (the selector of the canvas, which the input driver "EMSCRIPTEN" takes), GL_FRAMEBUFFER ("0"),
 *  GL_GETPROCADDRESS, GL_API ("GLES"), GL_VERSION ("3.0").
 *
 *  With a frequency in the video settings the window asks for a redraw on the pulses of the application, which on
 *  the web come once per display refresh; the browser shows the frame when the pulse returns.
 */
class GUIDRIVEREMSCRIPTEN_PUBLIC_CPP CEmscriptenWindowContext : public GUI::CWindowContext
{
    public:

    CEmscriptenWindowContext( void );

    virtual ~CEmscriptenWindowContext();

    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Shows or hides the pointer of the system over the canvas */
    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings ,
                     const CORE::CValueList* params           );

    void Shutdown( void );

    /** Changes the size of the drawing buffer of the canvas */
    bool Resize( GUI::UInt32 width, GUI::UInt32 height );

    /** Shows the canvas on the whole screen or in the page; false when the browser refuses */
    bool SetFullscreen( bool fullscreen );

    /** Makes the WebGL context current */
    bool MakeCurrent( void );

    /** Clears the canvas and lets the observers draw a frame */
    void RequestRedraw( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEmscriptenWindowContext( const CEmscriptenWindowContext& src );
    CEmscriptenWindowContext& operator=( const CEmscriptenWindowContext& src );

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    GUI::CString m_canvas;
    GUI::UInt32 m_width;
    GUI::UInt32 m_height;
    CORE::Int64 m_webglContext;     /**< EMSCRIPTEN_WEBGL_CONTEXT_HANDLE, 0 without a context */
    bool m_fullscreen;
    bool m_drivesRedraws;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CEmscriptenWindowContext, MT::CMutex > TEmscriptenWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H ? */
