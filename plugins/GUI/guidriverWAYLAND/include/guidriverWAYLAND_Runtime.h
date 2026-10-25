/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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


#ifndef GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#define GUCEF_GUIDRIVERWAYLAND_RUNTIME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

struct libdecor;
struct libdecor_frame;
struct libdecor_configuration;
struct libdecor_state;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      EGL                                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The few EGL constants the window needs, declared here so no EGL header is needed to build. The values are those of the
 *  Khronos registry.
 */
enum
{
    GUCEF_EGL_NONE                      = 0x3038 ,
    GUCEF_EGL_SURFACE_TYPE              = 0x3033 ,
    GUCEF_EGL_WINDOW_BIT                = 0x0004 ,
    GUCEF_EGL_RENDERABLE_TYPE           = 0x3040 ,
    GUCEF_EGL_OPENGL_BIT                = 0x0008 ,
    GUCEF_EGL_OPENGL_ES3_BIT            = 0x0040 ,
    GUCEF_EGL_RED_SIZE                  = 0x3024 ,
    GUCEF_EGL_GREEN_SIZE                = 0x3023 ,
    GUCEF_EGL_BLUE_SIZE                 = 0x3022 ,
    GUCEF_EGL_ALPHA_SIZE                = 0x3021 ,
    GUCEF_EGL_STENCIL_SIZE              = 0x3026 ,
    GUCEF_EGL_OPENGL_ES_API             = 0x30A0 ,
    GUCEF_EGL_OPENGL_API                = 0x30A2 ,
    GUCEF_EGL_CONTEXT_MAJOR_VERSION     = 0x3098 ,
    GUCEF_EGL_CONTEXT_MINOR_VERSION     = 0x30FB ,
    GUCEF_EGL_CONTEXT_OPENGL_PROFILE_MASK = 0x30FD ,
    GUCEF_EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT = 0x0001 ,
    GUCEF_EGL_PLATFORM_WAYLAND_KHR      = 0x31D8 ,
    GUCEF_EGL_TRUE                      = 1
};

/** The EGL entry points the window uses; EGLint is int, EGLBoolean unsigned int, every EGL handle a pointer */
struct SEglApi
{
    void* ( *getProcAddress )( const char* name );
    void* ( *getPlatformDisplay )( unsigned int platform, void* nativeDisplay, const intptr_t* attributes );     /**< EGL 1.5, NULL when absent */
    void* ( *getPlatformDisplayEXT )( unsigned int platform, void* nativeDisplay, const int* attributes );       /**< EGL_EXT_platform_base, NULL when absent */
    void* ( *getDisplay )( void* nativeDisplay );
    unsigned int ( *initialize )( void* display, int* major, int* minor );
    unsigned int ( *terminate )( void* display );
    unsigned int ( *bindApi )( unsigned int api );
    unsigned int ( *chooseConfig )( void* display, const int* attributes, void** configs, int configSize, int* configCount );
    void* ( *createContext )( void* display, void* config, void* shareContext, const int* attributes );
    unsigned int ( *destroyContext )( void* display, void* context );
    void* ( *createWindowSurface )( void* display, void* config, void* nativeWindow, const int* attributes );
    unsigned int ( *destroySurface )( void* display, void* surface );
    unsigned int ( *makeCurrent )( void* display, void* drawSurface, void* readSurface, void* context );
    unsigned int ( *swapBuffers )( void* display, void* surface );
    unsigned int ( *swapInterval )( void* display, int interval );
    int ( *getError )( void );
};
typedef struct SEglApi SEglApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      libdecor                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The callbacks libdecor wants for the library as a whole; the slots after the first are reserved by the library and stay empty */
struct SLibdecorInterface
{
    void ( *error )( struct libdecor* context, int error, const char* message );
    void ( *reserved[ 10 ] )( void );
};

/** The callbacks libdecor wants for a window; the slots after the fourth are reserved by the library and stay empty */
struct SLibdecorFrameInterface
{
    void ( *configure )( struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* userData );
    void ( *close )( struct libdecor_frame* frame, void* userData );
    void ( *commit )( struct libdecor_frame* frame, void* userData );
    void ( *dismiss_popup )( struct libdecor_frame* frame, const char* seatName, void* userData );
    void ( *reserved[ 10 ] )( void );
};

/** The libdecor entry points (libdecor-0.so.0) */
struct SLibdecorApi
{
    struct libdecor* ( *create )( struct wl_display* display, const struct SLibdecorInterface* iface );
    void ( *unref )( struct libdecor* context );
    int ( *dispatch )( struct libdecor* context, int timeout );
    struct libdecor_frame* ( *decorate )( struct libdecor* context, struct wl_surface* surface, const struct SLibdecorFrameInterface* iface, void* userData );
    void ( *frame_unref )( struct libdecor_frame* frame );
    void ( *frame_set_title )( struct libdecor_frame* frame, const char* title );
    void ( *frame_set_app_id )( struct libdecor_frame* frame, const char* appId );
    void ( *frame_set_min_content_size )( struct libdecor_frame* frame, int width, int height );
    void ( *frame_set_max_content_size )( struct libdecor_frame* frame, int width, int height );
    void ( *frame_set_fullscreen )( struct libdecor_frame* frame, struct wl_output* output );
    void ( *frame_unset_fullscreen )( struct libdecor_frame* frame );
    void ( *frame_set_maximized )( struct libdecor_frame* frame );
    void ( *frame_unset_maximized )( struct libdecor_frame* frame );
    void ( *frame_set_visibility )( struct libdecor_frame* frame, bool visible );
    void ( *frame_map )( struct libdecor_frame* frame );
    void ( *frame_commit )( struct libdecor_frame* frame, struct libdecor_state* state, struct libdecor_configuration* configuration );
    bool ( *configuration_get_content_size )( struct libdecor_configuration* configuration, struct libdecor_frame* frame, int* width, int* height );
    bool ( *configuration_get_window_state )( struct libdecor_configuration* configuration, int* windowState );
    struct libdecor_state* ( *state_new )( int width, int height );
    void ( *state_free )( struct libdecor_state* state );
};
typedef struct SLibdecorApi SLibdecorApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE LIBRARIES AS A WHOLE                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The system libraries the window needs besides the connection: EGL and the Wayland to EGL window glue are required,
 *  libdecor is optional. A test makes this with functions of its own.
 */
struct GUIDRIVERWAYLAND_PUBLIC_CPP SWaylandGuiRuntime
{
    SEglApi egl;
    SWaylandEglWindowApi eglWindow;
    SLibdecorApi libdecor;
    bool hasEgl;
    bool hasEglWindow;
    bool hasLibdecor;

    SWaylandGuiRuntime( void );
};

/** Loads libEGL.so.1, libwayland-egl.so.1 and libdecor-0.so.0 when they are there; false when a required one is missing, with the name of what is missing in 'problem' */
GUIDRIVERWAYLAND_PUBLIC_CPP bool
LoadWaylandGuiRuntime( SWaylandGuiRuntime& runtime, const char** missing );

/** The runtime of the process, loaded once */
GUIDRIVERWAYLAND_PUBLIC_CPP const SWaylandGuiRuntime&
GetWaylandGuiRuntime( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_RUNTIME_H ? */
