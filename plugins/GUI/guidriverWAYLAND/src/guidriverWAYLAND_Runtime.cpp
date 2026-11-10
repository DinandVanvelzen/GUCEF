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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#include "pluginglueWAYLAND_CWaylandRuntime.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H ? */

#include "guidriverWAYLAND_Runtime.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SWaylandGuiRuntime::SWaylandGuiRuntime( void )
    : hasEgl( false )
    , hasEglWindow( false )
    , hasLibdecor( false )
{GUCEF_TRACE;

    memset( &egl, 0, sizeof( egl ) );
    memset( &eglWindow, 0, sizeof( eglWindow ) );
    memset( &libdecor, 0, sizeof( libdecor ) );
}

/*-------------------------------------------------------------------------*/

namespace {

template< typename T >
void
Fill( T& slot, void* module, const char* name )
{
    slot = (T) PLUGINGLUE::WAYLAND::CWaylandRuntime::GetSymbol( module, name );
}

}

/*-------------------------------------------------------------------------*/

bool
LoadWaylandGuiRuntime( SWaylandGuiRuntime& runtime, const char** missing )
{GUCEF_TRACE;

    using PLUGINGLUE::WAYLAND::CWaylandRuntime;
    if ( GUCEF_NULL != missing )
        *missing = GUCEF_NULL;

    void* egl = CWaylandRuntime::LoadLibraryOnce( "libEGL.so.1" );
    if ( GUCEF_NULL != egl )
    {
        Fill( runtime.egl.getProcAddress, egl, "eglGetProcAddress" );
        Fill( runtime.egl.getPlatformDisplay, egl, "eglGetPlatformDisplay" );
        Fill( runtime.egl.getPlatformDisplayEXT, egl, "eglGetPlatformDisplayEXT" );
        Fill( runtime.egl.getDisplay, egl, "eglGetDisplay" );
        Fill( runtime.egl.initialize, egl, "eglInitialize" );
        Fill( runtime.egl.terminate, egl, "eglTerminate" );
        Fill( runtime.egl.bindApi, egl, "eglBindAPI" );
        Fill( runtime.egl.chooseConfig, egl, "eglChooseConfig" );
        Fill( runtime.egl.createContext, egl, "eglCreateContext" );
        Fill( runtime.egl.destroyContext, egl, "eglDestroyContext" );
        Fill( runtime.egl.createWindowSurface, egl, "eglCreateWindowSurface" );
        Fill( runtime.egl.destroySurface, egl, "eglDestroySurface" );
        Fill( runtime.egl.makeCurrent, egl, "eglMakeCurrent" );
        Fill( runtime.egl.swapBuffers, egl, "eglSwapBuffers" );
        Fill( runtime.egl.swapInterval, egl, "eglSwapInterval" );
        Fill( runtime.egl.getError, egl, "eglGetError" );
        runtime.hasEgl = GUCEF_NULL != runtime.egl.getProcAddress && ( GUCEF_NULL != runtime.egl.getPlatformDisplay || GUCEF_NULL != runtime.egl.getPlatformDisplayEXT || GUCEF_NULL != runtime.egl.getDisplay ) &&
                         GUCEF_NULL != runtime.egl.initialize && GUCEF_NULL != runtime.egl.bindApi && GUCEF_NULL != runtime.egl.chooseConfig && GUCEF_NULL != runtime.egl.createContext &&
                         GUCEF_NULL != runtime.egl.createWindowSurface && GUCEF_NULL != runtime.egl.makeCurrent && GUCEF_NULL != runtime.egl.swapBuffers &&
                         GUCEF_NULL != runtime.egl.swapInterval && GUCEF_NULL != runtime.egl.destroyContext && GUCEF_NULL != runtime.egl.destroySurface;
    }
    runtime.hasEglWindow = CWaylandRuntime::LoadEglWindowApi( runtime.eglWindow );

    void* decor = CWaylandRuntime::LoadLibraryOnce( "libdecor-0.so.0" );
    if ( GUCEF_NULL != decor )
    {
        Fill( runtime.libdecor.create, decor, "libdecor_new" );
        Fill( runtime.libdecor.unref, decor, "libdecor_unref" );
        Fill( runtime.libdecor.dispatch, decor, "libdecor_dispatch" );
        Fill( runtime.libdecor.decorate, decor, "libdecor_decorate" );
        Fill( runtime.libdecor.frame_unref, decor, "libdecor_frame_unref" );
        Fill( runtime.libdecor.frame_set_title, decor, "libdecor_frame_set_title" );
        Fill( runtime.libdecor.frame_set_app_id, decor, "libdecor_frame_set_app_id" );
        Fill( runtime.libdecor.frame_set_min_content_size, decor, "libdecor_frame_set_min_content_size" );
        Fill( runtime.libdecor.frame_set_max_content_size, decor, "libdecor_frame_set_max_content_size" );
        Fill( runtime.libdecor.frame_set_fullscreen, decor, "libdecor_frame_set_fullscreen" );
        Fill( runtime.libdecor.frame_unset_fullscreen, decor, "libdecor_frame_unset_fullscreen" );
        Fill( runtime.libdecor.frame_set_maximized, decor, "libdecor_frame_set_maximized" );
        Fill( runtime.libdecor.frame_unset_maximized, decor, "libdecor_frame_unset_maximized" );
        Fill( runtime.libdecor.frame_set_visibility, decor, "libdecor_frame_set_visibility" );
        Fill( runtime.libdecor.frame_map, decor, "libdecor_frame_map" );
        Fill( runtime.libdecor.frame_commit, decor, "libdecor_frame_commit" );
        Fill( runtime.libdecor.configuration_get_content_size, decor, "libdecor_configuration_get_content_size" );
        Fill( runtime.libdecor.configuration_get_window_state, decor, "libdecor_configuration_get_window_state" );
        Fill( runtime.libdecor.state_new, decor, "libdecor_state_new" );
        Fill( runtime.libdecor.state_free, decor, "libdecor_state_free" );
        runtime.hasLibdecor = GUCEF_NULL != runtime.libdecor.create && GUCEF_NULL != runtime.libdecor.unref && GUCEF_NULL != runtime.libdecor.dispatch &&
                              GUCEF_NULL != runtime.libdecor.decorate && GUCEF_NULL != runtime.libdecor.frame_unref && GUCEF_NULL != runtime.libdecor.frame_commit &&
                              GUCEF_NULL != runtime.libdecor.frame_map && GUCEF_NULL != runtime.libdecor.configuration_get_content_size &&
                              GUCEF_NULL != runtime.libdecor.configuration_get_window_state && GUCEF_NULL != runtime.libdecor.state_new &&
                              GUCEF_NULL != runtime.libdecor.state_free && GUCEF_NULL != runtime.libdecor.frame_set_title;
    }

    if ( !runtime.hasEgl )
    {
        if ( GUCEF_NULL != missing )
            *missing = "libEGL.so.1";
        return false;
    }
    if ( !runtime.hasEglWindow )
    {
        if ( GUCEF_NULL != missing )
            *missing = "libwayland-egl.so.1";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const SWaylandGuiRuntime&
GetWaylandGuiRuntime( void )
{GUCEF_TRACE;

    static SWaylandGuiRuntime runtime;
    static bool loaded = false;
    if ( !loaded )
    {
        loaded = true;
        LoadWaylandGuiRuntime( runtime, GUCEF_NULL );
    }
    return runtime;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
