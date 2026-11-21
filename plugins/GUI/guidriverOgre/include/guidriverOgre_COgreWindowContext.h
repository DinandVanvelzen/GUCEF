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

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreRenderTargetListener.h>
#include <OgreTexture.h>

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#include "guidriverOgre_CIOgreWindowOverlay.h"
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */

#ifndef GUIDRIVEROGRE_COGRESURFACESTATE_H
#include "guidriverOgre_COgreSurfaceState.h"
#define GUIDRIVEROGRE_COGRESURFACESTATE_H
#endif /* GUIDRIVEROGRE_COGRESURFACESTATE_H ? */

#include "OgrePrerequisites.h"   /* forward declares Ogre::Root, which lives in a build dependent namespace of Ogre */

namespace Ogre { class RenderWindow; class Viewport; class SceneManager; }

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
 *  A window of the window backend "Ogre": an Ogre render window on a window of a platform window backend.
 *
 *  The platform window is made with the window parameter GraphicsApi=None, so it has no graphics context, and the
 *  render window is made from its native handles (COgrePlatformWindowSupport: the X11 window, the wl_display and
 *  wl_surface, the HWND, the ANativeWindow, the canvas). Size, close requests, activation, the cursor and the
 *  properties of the platform window (the native handles that input drivers attach to) are passed on. The platform
 *  backend is the window parameter "PlatformBackend", by default the one of the build platform (XWinGL on Linux, Win32
 *  on Windows, AndroidGLES on Android, Emscripten on the web); "Wayland" on Linux with the Wayland flavour of the render
 *  system, or "Offscreen" for a render window of Ogre's own without a platform window (Ogre's headless EGL surface
 *  where there is no display server).
 *
 *  When the platform window loses its native surface (WindowContextSurfaceLostEvent, an Android app in the background)
 *  Ogre is told, and no frames are drawn until the surface comes back (WindowContextSurfaceRestoredEvent), when Ogre gets
 *  the new native window; the render window, the Root and the resources stay (COgreSurfaceState).
 *
 *  Frames are paced by the platform window. On each of its redraws the window draws one frame, in this order:
 *  1. Root::_fireFrameStarted(): the frame listeners of the application;
 *  2. its own WindowContextRedrawEvent: the GUI contexts of the window update and prepare their frame;
 *  3. RenderWindow::update( false ): the viewports of the application, then the overlays (CIOgreWindowOverlay) from
 *     the lowest order up; without viewports the window is cleared to black first;
 *  4. Root::_fireFrameRenderingQueued() and RenderWindow::swapBuffers();
 *  5. Root::_fireFrameEnded(), which also runs the main thread tasks of Ogre's work queue.
 *  Applications do not call Root::renderOneFrame() or Root::startRendering().
 *
 *  The window reports the property GRAPHICS_API "Ogre" and offers no GL properties, so GUI drivers for OpenGL do not
 *  draw in it.
 */
class GUIDRIVEROGRE_PUBLIC_CPP COgreWindowContext : public GUI::CWindowContext       ,
                                                    private Ogre::RenderTargetListener
{
    public:

    static const CORE::CString ClassTypeName;                   /**< what GetClassTypeName() gives, to recognise the windows of this backend */
    static const CORE::CString PlatformBackendParameterName;    /**< "PlatformBackend" */
    static const CORE::CString OffscreenPlatformBackend;        /**< "Offscreen" */
    static const CORE::CString GraphicsApiName;                 /**< "Ogre": the GRAPHICS_API property of the windows */

    COgreWindowContext( void );

    virtual ~COgreWindowContext();

    /**
     *  Makes the platform window and the render window. The Ogre Root is the one of pluginglueOGRE, initialised here
     *  with the render system of its configuration when it is not initialised yet. 'params' may hold
     *  "PlatformBackend" and the parameters of the platform backend (X, Y). The video settings give the size, full
     *  screen, vsync, the frequency of the frames and the anti aliasing (Ogre's FSAA)
     */
    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings ,
                     const CORE::CValueList* params           );

    /** Destroys the render window and the platform window; the overlays stay registered until they unregister */
    void Shutdown( void );

    /** The render window, NULL before Initialize() and after Shutdown() */
    Ogre::RenderWindow* GetOgreRenderWindow( void ) const;

    /** The Root the render window belongs to */
    Ogre::Root* GetOgreRoot( void ) const;

    /** The window of the platform backend, NULL for an offscreen window */
    GUI::TWindowContextPtr GetPlatformWindowContext( void ) const;

    /** "XWinGL", "Offscreen", ... */
    const CORE::CString& GetPlatformBackendName( void ) const;

    /** Full screen and size through the platform window, vsync through the render window */
    bool ApplyVideoSettings( const GUI::CVideoSettings& videoSettings );

    /** The video settings the window has now: size, full screen, vsync, anti aliasing and frequency as Ogre reports them */
    GUI::CVideoSettings GetVideoSettings( void ) const;

    /** The overlay draws over the viewports in every frame, until it unregisters. It must unregister before it goes */
    void RegisterOverlay( CIOgreWindowOverlay* overlay );

    void UnregisterOverlay( CIOgreWindowOverlay* overlay );

    UInt32 GetOverlayCount( void ) const;

    /**
     *  Draws a frame now, as a redraw of the platform window does. With 'capture' the picture of the window is read
     *  back before it is presented (RGBA, rows from the top). False when there is no render window or reading failed
     */
    bool RenderFrame( IMAGE::TPixelMapPtr* capture = GUCEF_NULL );

    /**
     *  Draws the overlay alone into a render texture of the size given and reads it back (RGBA, rows from the top).
     *  The texture starts as opaque black when 'opaque', else as transparent black. The render texture is kept for the
     *  next capture of the same size
     */
    bool CaptureOverlay( CIOgreWindowOverlay& overlay ,
                         UInt32 width                 ,
                         UInt32 height                ,
                         bool opaque                  ,
                         IMAGE::TPixelMapPtr& pixels  );

    /** The number of frames drawn since Initialize() */
    UInt64 GetFrameCount( void ) const;

    /** Whether the render window has its native surface, and how often it was lost and adopted again */
    const COgreSurfaceState& GetSurfaceState( void ) const;

    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  GRAPHICS_API is "Ogre", GL_* properties are empty, "Ogre::RenderWindow" and "Ogre::SceneManager" (the scene manager of
     *  the camera of the last viewport, NULL while there is none) are pointers for GUI drivers that draw through Ogre scene
     *  rendering, RENDERER is the renderer the window draws with ("Vulkan", "OpenGL", ..., COgreRootSetup::GetRendererName()),
     *  everything else is the platform window's (WINDOW, DISPLAY, ...)
     */
    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;

    /** The platform window shows the shape, the Ogre window is only drawn into it */
    virtual bool SetSystemCursorShape( GUI::TCursorShape shape ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< COgreWindowContext > TEventCallback;

    struct SOverlayEntry
    {
        CIOgreWindowOverlay* overlay;
        UInt64 sequence;
    };
    typedef GUCEF::vector< SOverlayEntry > TOverlayVector;

    void OnPlatformRedraw( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlatformSize( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlatformActivate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlatformCloseRequest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlatformSurfaceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlatformSurfaceRestored( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    virtual void postRenderTargetUpdate( const Ogre::RenderTargetEvent& evt ) GUCEF_VIRTUAL_OVERRIDE;

    bool CreatePlatformWindow( const GUI::CString& title, const GUI::CVideoSettings& videoSettings, const CORE::CValueList* params );
    bool CreateRenderWindow( const GUI::CString& title, const GUI::CVideoSettings& videoSettings );
    void DrawOverlays( Ogre::Viewport& viewport, Ogre::SceneManager* sceneManager, bool isCapture );
    void SortOverlays( void );
    static bool ReadPixels( Ogre::RenderTarget& target, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels );

    COgreWindowContext( const COgreWindowContext& src );            /**< private because: must be unique */
    COgreWindowContext& operator=( const COgreWindowContext& src ); /**< private because: must be unique */

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    CORE::CString m_platformBackendName;
    GUI::TWindowManagerBackendPtr m_platformBackend;
    GUI::TWindowContextPtr m_platformWindow;
    Ogre::RenderWindow* m_renderWindow;
    Ogre::Viewport* m_overlayViewport;      /**< the whole window, for the overlays; not one of the window's viewports */
    Ogre::TexturePtr m_captureTexture;
    Ogre::Viewport* m_captureViewport;
    TOverlayVector m_overlays;
    UInt64 m_overlaySequence;
    UInt64 m_frameCount;
    bool m_inFrame;
    GUI::CVideoSettings m_videoSettings;
    COgreSurfaceState m_surfaceState;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< COgreWindowContext, MT::CMutex > TOgreWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */
