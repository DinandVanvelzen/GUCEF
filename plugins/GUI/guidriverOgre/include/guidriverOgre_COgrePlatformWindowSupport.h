/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H
#define GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreCommon.h>

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVEROGRE_MACROS_H
#include "guidriverOgre_macros.h"
#define GUIDRIVEROGRE_MACROS_H
#endif /* GUIDRIVEROGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What an Ogre render window on a window of a platform window backend depends on per platform, as functions without
 *  state, so that the choices of every platform can be checked on any build platform.
 *
 *  The platform backend of a window is the window parameter "PlatformBackend" when it is given, else the backend of the
 *  build platform: XWinGL on Linux, Win32 on Windows, AndroidGLES on Android, Emscripten on the web, Offscreen elsewhere.
 *  Wayland is chosen with the parameter: Ogre 1.x selects X11 or Wayland when it is compiled, so the Wayland windows
 *  need the render system modules of the Wayland flavour (OgreRenderSystem_GL3Plus_Wayland,
 *  OgreRenderSystem_GLES2_Wayland), which an application lists instead of the X11 ones.
 *
 *  The parameters of Root::createRenderWindow() come from the properties of the platform window:
 *
 *      XWinGL       externalWindowHandle = WINDOW (the X11 window; Ogre makes a child window in it)
 *      Wayland      externalWlDisplay = DISPLAY, externalWlSurface = WL_SURFACE (Ogre makes its wl_egl_window on the
 *                   surface); vsync is off for Ogre, the frame callbacks of the platform window pace the frames
 *      Win32        externalWindowHandle = WINDOW (the HWND; Direct3D 11 makes its swap chain, OpenGL its WGL context on it)
 *      AndroidGLES  externalWindowHandle = WINDOW (the ANativeWindow), androidConfig = ANDROID_CONFIG, preserveContext:
 *                   the EGL context, and with it the resources, outlives the native window when the app is paused
 *      Emscripten   externalWindowHandle = WINDOW (the CSS selector of the canvas)
 *      Offscreen    none: Ogre's own headless surface
 *
 *  and from the video settings: title, vsync, FSAA (anti aliasing factor above 1), displayFrequency. The render window
 *  is never full screen in Ogre's sense: full screen is the platform window's.
 */
class GUIDRIVEROGRE_PUBLIC_CPP COgrePlatformWindowSupport
{
    public:

    static const CORE::CString XWinGLBackend;           /**< "XWinGL" */
    static const CORE::CString WaylandBackend;          /**< "Wayland" */
    static const CORE::CString Win32Backend;            /**< "Win32" */
    static const CORE::CString AndroidGLESBackend;      /**< "AndroidGLES" */
    static const CORE::CString EmscriptenBackend;       /**< "Emscripten" */
    static const CORE::CString OffscreenBackend;        /**< "Offscreen": no platform window */
    static const CORE::CString PlatformBackendParameterName;   /**< "PlatformBackend" */

    /** How the render window follows a new size of the platform window */
    enum EResizeMethod
    {
        RESIZEMETHOD_RESIZE     = 0,    /**< RenderWindow::resize() with the size of the platform window: Ogre sizes what it owns (the X11 child window, the wl_egl_window, an offscreen surface) */
        RESIZEMETHOD_FROM_NATIVE        /**< RenderWindow::windowMovedOrResized(): Ogre reads the size of the native window it was given (HWND, ANativeWindow, canvas) */
    };

    /** The platform backend of the build platform */
    static CORE::CString GetDefaultPlatformBackend( void );

    /** The platform backend of a platform, by its GUCEF_PLATFORM_* number */
    static CORE::CString GetDefaultPlatformBackendForPlatform( UInt32 gucefPlatform );

    /**
     *  The platform backend the window parameters ask for, written as the backend names itself when it is one of the
     *  known ones in any letter case, as it is given otherwise; 'defaultBackend' when the parameters name none
     */
    static CORE::CString SelectPlatformBackend( const CORE::CValueList* params      ,
                                                const CORE::CString& defaultBackend );

    /** Whether Ogre render windows can be made on windows of the backend */
    static bool IsKnownPlatformBackend( const CORE::CString& backendName );

    /** Whether the platform window paces the frames (true for every backend but Offscreen) */
    static bool DoesPlatformPaceFrames( const CORE::CString& backendName );

    static EResizeMethod GetResizeMethod( const CORE::CString& backendName );

    /** Whether the backend can lose the native surface while the window stays (AndroidGLES): Ogre is told, the window is kept */
    static bool CanLoseSurface( const CORE::CString& backendName );

    /** The period of the pulses of a window that paces itself, 0 for no pulses (frequency 0) */
    static UInt32 GetPulseIntervalInMs( UInt32 frequency );

    /** The parameters of the video settings: title, vsync, FSAA, displayFrequency */
    static void AddVideoSettingParams( const CORE::CString& backendName          ,
                                       const GUI::CVideoSettings& videoSettings ,
                                       const CORE::CString& title               ,
                                       Ogre::NameValuePairList& miscParams      );

    /**
     *  The parameters of the native handles, from the properties of the platform window. False with the reason in
     *  'problem' when the backend is unknown or a handle is missing
     */
    static bool AddNativeWindowParams( const CORE::CString& backendName       ,
                                       const GUI::CWindowContext& platformWindow ,
                                       Ogre::NameValuePairList& miscParams    ,
                                       CORE::CString& problem                 );

    /** Ogre takes window handles as decimal numbers in a string; a pointer property is converted, NULL gives "0" */
    static Ogre::String PointerPropertyToHandle( const CORE::CString& pointerProperty );

    private:

    COgrePlatformWindowSupport( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H ? */
