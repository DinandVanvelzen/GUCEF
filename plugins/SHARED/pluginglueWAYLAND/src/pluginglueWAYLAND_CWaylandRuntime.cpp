/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pluginglueWAYLAND_CWaylandRuntime.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Made once and kept: the plugins that use the library can unload after the statics of this library are gone */
struct SClientApiState
{
    MT::CMutex lock;
    SWaylandApi api;
    bool tried;
    bool loaded;

    SClientApiState( void ) : lock(), api(), tried( false ), loaded( false ) {}
};

SClientApiState&
ClientApiState( void )
{
    static SClientApiState* state = new SClientApiState();
    return *state;
}

/** A listener of the code that lacks a function for an event is a programming error: it is not handed to libwayland, which would abort the process on that event */
void
ReportRejectedListener( const char* interfaceName )
{
    const char* name = GUCEF_NULL != interfaceName ? interfaceName : "?";
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWaylandRuntime: The listener for " << name << " has no function for every event: it is not used" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void*
CWaylandRuntime::LoadLibraryOnce( const char* fileName )
{GUCEF_TRACE;

    // the system loader keeps one copy of a library per name: asking again gives the same module
    void* module = CORE::LoadModuleDynamicly( fileName );
    if ( GUCEF_NULL == module )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandRuntime: " << fileName << " is not available" );
    return module;
}

/*-------------------------------------------------------------------------*/

void*
CWaylandRuntime::GetSymbol( void* module, const char* name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == module )
        return GUCEF_NULL;
    return CORE::GetFunctionAddress( module, name, 0 ).objPtr;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandRuntime::LoadClientApi( SWaylandApi& api, CORE::CString* problem )
{GUCEF_TRACE;

    void* lib = LoadLibraryOnce( "libwayland-client.so.0" );
    if ( GUCEF_NULL == lib )
    {
        if ( GUCEF_NULL != problem )
            *problem = "libwayland-client.so.0 is not installed";
        return false;
    }

    api.display_connect = (struct wl_display* ( * )( const char* )) GetSymbol( lib, "wl_display_connect" );
    api.display_disconnect = (void ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_disconnect" );
    api.display_get_fd = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_get_fd" );
    api.display_flush = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_flush" );
    api.display_dispatch_pending = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_dispatch_pending" );
    api.display_prepare_read = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_prepare_read" );
    api.display_read_events = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_read_events" );
    api.display_cancel_read = (void ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_cancel_read" );
    api.display_roundtrip = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_roundtrip" );
    api.display_get_error = (int ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_get_error" );
    api.proxy_marshal_flags = (struct wl_proxy* ( * )( struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t, uint32_t, ... )) GetSymbol( lib, "wl_proxy_marshal_flags" );
    api.proxy_add_listener = (int ( * )( struct wl_proxy*, void ( ** )( void ), void* )) GetSymbol( lib, "wl_proxy_add_listener" );
    api.proxy_destroy = (void ( * )( struct wl_proxy* )) GetSymbol( lib, "wl_proxy_destroy" );
    api.proxy_get_version = (uint32_t ( * )( struct wl_proxy* )) GetSymbol( lib, "wl_proxy_get_version" );
    api.proxy_set_user_data = (void ( * )( struct wl_proxy*, void* )) GetSymbol( lib, "wl_proxy_set_user_data" );
    api.proxy_get_user_data = (void* ( * )( struct wl_proxy* )) GetSymbol( lib, "wl_proxy_get_user_data" );
    api.proxy_get_id = (uint32_t ( * )( struct wl_proxy* )) GetSymbol( lib, "wl_proxy_get_id" );
    api.display_create_queue = (struct wl_event_queue* ( * )( struct wl_display* )) GetSymbol( lib, "wl_display_create_queue" );
    api.event_queue_destroy = (void ( * )( struct wl_event_queue* )) GetSymbol( lib, "wl_event_queue_destroy" );
    api.proxy_set_queue = (void ( * )( struct wl_proxy*, struct wl_event_queue* )) GetSymbol( lib, "wl_proxy_set_queue" );
    api.display_dispatch_queue_pending = (int ( * )( struct wl_display*, struct wl_event_queue* )) GetSymbol( lib, "wl_display_dispatch_queue_pending" );
    api.display_prepare_read_queue = (int ( * )( struct wl_display*, struct wl_event_queue* )) GetSymbol( lib, "wl_display_prepare_read_queue" );
    api.listener_rejected = &ReportRejectedListener;

    if ( GUCEF_NULL == api.proxy_marshal_flags )
    {
        if ( GUCEF_NULL != problem )
            *problem = "libwayland-client.so.0 is older than version 1.19 (it has no wl_proxy_marshal_flags)";
        return false;
    }
    const bool complete = GUCEF_NULL != api.display_connect && GUCEF_NULL != api.display_disconnect && GUCEF_NULL != api.display_get_fd &&
                          GUCEF_NULL != api.display_flush && GUCEF_NULL != api.display_dispatch_pending && GUCEF_NULL != api.display_prepare_read &&
                          GUCEF_NULL != api.display_read_events && GUCEF_NULL != api.display_cancel_read && GUCEF_NULL != api.display_roundtrip &&
                          GUCEF_NULL != api.display_get_error && GUCEF_NULL != api.proxy_add_listener && GUCEF_NULL != api.proxy_destroy &&
                          GUCEF_NULL != api.proxy_get_version && GUCEF_NULL != api.proxy_set_user_data && GUCEF_NULL != api.proxy_get_user_data &&
                          GUCEF_NULL != api.proxy_get_id;
    if ( !complete && GUCEF_NULL != problem )
        *problem = "libwayland-client.so.0 lacks functions that are part of its interface";
    return complete;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandRuntime::LoadCursorApi( SWaylandCursorApi& api )
{GUCEF_TRACE;

    void* lib = LoadLibraryOnce( "libwayland-cursor.so.0" );
    api.theme_load = (struct wl_cursor_theme* ( * )( const char*, int, struct wl_shm* )) GetSymbol( lib, "wl_cursor_theme_load" );
    api.theme_destroy = (void ( * )( struct wl_cursor_theme* )) GetSymbol( lib, "wl_cursor_theme_destroy" );
    api.theme_get_cursor = (struct wl_cursor* ( * )( struct wl_cursor_theme*, const char* )) GetSymbol( lib, "wl_cursor_theme_get_cursor" );
    api.image_get_buffer = (struct wl_buffer* ( * )( struct wl_cursor_image* )) GetSymbol( lib, "wl_cursor_image_get_buffer" );
    return GUCEF_NULL != api.theme_load && GUCEF_NULL != api.theme_destroy && GUCEF_NULL != api.theme_get_cursor && GUCEF_NULL != api.image_get_buffer;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandRuntime::LoadEglWindowApi( SWaylandEglWindowApi& api )
{GUCEF_TRACE;

    void* lib = LoadLibraryOnce( "libwayland-egl.so.1" );
    api.window_create = (struct wl_egl_window* ( * )( struct wl_surface*, int, int )) GetSymbol( lib, "wl_egl_window_create" );
    api.window_destroy = (void ( * )( struct wl_egl_window* )) GetSymbol( lib, "wl_egl_window_destroy" );
    api.window_resize = (void ( * )( struct wl_egl_window*, int, int, int, int )) GetSymbol( lib, "wl_egl_window_resize" );
    api.window_get_attached_size = (void ( * )( struct wl_egl_window*, int*, int* )) GetSymbol( lib, "wl_egl_window_get_attached_size" );
    return GUCEF_NULL != api.window_create && GUCEF_NULL != api.window_destroy && GUCEF_NULL != api.window_resize;
}

/*-------------------------------------------------------------------------*/

const SWaylandApi*
CWaylandRuntime::GetClientApi( void )
{GUCEF_TRACE;

    SClientApiState& state = ClientApiState();
    MT::CScopeMutex lock( state.lock );
    if ( !state.tried )
    {
        state.tried = true;
        CORE::CString problem;
        state.loaded = LoadClientApi( state.api, &problem );
        if ( !state.loaded )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandRuntime: Wayland is not usable: " << problem );
    }
    return state.loaded ? &state.api : GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
