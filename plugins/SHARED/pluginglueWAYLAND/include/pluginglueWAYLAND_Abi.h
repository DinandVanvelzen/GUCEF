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

#ifndef GUCEF_PLUGINGLUE_WAYLAND_ABI_H
#define GUCEF_PLUGINGLUE_WAYLAND_ABI_H

/*
 *  The binary interface of libwayland-client, libwayland-cursor and libwayland-egl as far as the plugins use it, declared
 *  here so the plugins build without the Wayland development packages. The libraries are loaded when asked for (dlopen)
 *  and the entry points kept in the structures below: a test replaces the structure with functions of its own. This file is
 *  plain C, the generated protocol code includes it.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      WIRE TYPES                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

struct wl_proxy;
struct wl_display;
struct wl_interface;
struct wl_event_queue;

struct wl_message
{
    const char* name;
    const char* signature;
    const struct wl_interface** types;
};

struct wl_interface
{
    const char* name;
    int version;
    int method_count;
    const struct wl_message* methods;
    int event_count;
    const struct wl_message* events;
};

struct wl_array
{
    size_t size;
    size_t alloc;
    void* data;
};

/** Signed 24.8 fixed point */
typedef int32_t wl_fixed_t;

#define GUCEF_WL_MARSHAL_FLAG_DESTROY 1

#if defined( __GNUC__ )
  #define GUCEF_WL_PROTO_API extern __attribute__((visibility("default")))
#else
  #define GUCEF_WL_PROTO_API extern
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      libwayland-client                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SWaylandApi
{
    struct wl_display* ( *display_connect )( const char* name );
    void ( *display_disconnect )( struct wl_display* display );
    int ( *display_get_fd )( struct wl_display* display );
    int ( *display_flush )( struct wl_display* display );
    int ( *display_dispatch_pending )( struct wl_display* display );
    int ( *display_prepare_read )( struct wl_display* display );
    int ( *display_read_events )( struct wl_display* display );
    void ( *display_cancel_read )( struct wl_display* display );
    int ( *display_roundtrip )( struct wl_display* display );
    int ( *display_get_error )( struct wl_display* display );

    /** Since libwayland 1.19: the one marshalling call all requests use */
    struct wl_proxy* ( *proxy_marshal_flags )( struct wl_proxy* proxy, uint32_t opcode, const struct wl_interface* iface, uint32_t version, uint32_t flags, ... );
    int ( *proxy_add_listener )( struct wl_proxy* proxy, void ( **implementation )( void ), void* data );
    void ( *proxy_destroy )( struct wl_proxy* proxy );
    uint32_t ( *proxy_get_version )( struct wl_proxy* proxy );
    void ( *proxy_set_user_data )( struct wl_proxy* proxy, void* userData );
    void* ( *proxy_get_user_data )( struct wl_proxy* proxy );
    uint32_t ( *proxy_get_id )( struct wl_proxy* proxy );

    /**
     *  Optional: event queues of their own. Requests that are answered with an event while the code is itself inside the dispatch
     *  of the default queue (an activation token asked for in the handler of a click) put the answer on a queue of their own,
     *  so that waiting for it dispatches nothing else. Either all five are there or none.
     */
    struct wl_event_queue* ( *display_create_queue )( struct wl_display* display );
    void ( *event_queue_destroy )( struct wl_event_queue* queue );
    void ( *proxy_set_queue )( struct wl_proxy* proxy, struct wl_event_queue* queue );
    int ( *display_dispatch_queue_pending )( struct wl_display* display, struct wl_event_queue* queue );
    int ( *display_prepare_read_queue )( struct wl_display* display, struct wl_event_queue* queue );

    /** Optional: told the name of the interface when add_listener refuses a listener that has no function for an event the proxy can get */
    void ( *listener_rejected )( const char* interfaceName );
};
typedef struct SWaylandApi SWaylandApi;

/**
 *  Whether a listener (an array of one function per event, in the order of the events of the interface) has a function for
 *  every event that a proxy of the given version can receive. libwayland-client aborts the whole process when an event comes
 *  for which the listener has no function ("listener function for opcode N of wl_pointer is NULL"), so a listener that is
 *  short or has a hole is never handed to it.
 */
static inline int
gucef_wl_listener_is_complete( const struct wl_interface* iface, unsigned int slots, const void* listener, uint32_t version )
{
    const void* const* functions = (const void* const*) listener;
    int event;
    if ( 0 == iface || 0 == listener )
        return 0;
    for ( event = 0; event < iface->event_count; ++event )
    {
        /* the signature of a message starts with the version it was added in */
        const char* signature = iface->events[ event ].signature;
        uint32_t since = 0;
        while ( *signature >= '0' && *signature <= '9' )
            since = since * 10 + (uint32_t) ( *signature++ - '0' );
        if ( since > version )
            continue;
        if ( (unsigned int) event >= slots || 0 == functions[ event ] )
            return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      libwayland-cursor                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

struct wl_cursor_theme;
struct wl_buffer;
struct wl_shm;

struct wl_cursor_image
{
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};

struct wl_cursor
{
    unsigned int image_count;
    struct wl_cursor_image** images;
    char* name;
};

struct SWaylandCursorApi
{
    struct wl_cursor_theme* ( *theme_load )( const char* name, int size, struct wl_shm* shm );
    void ( *theme_destroy )( struct wl_cursor_theme* theme );
    struct wl_cursor* ( *theme_get_cursor )( struct wl_cursor_theme* theme, const char* name );
    struct wl_buffer* ( *image_get_buffer )( struct wl_cursor_image* image );
};
typedef struct SWaylandCursorApi SWaylandCursorApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      libwayland-egl                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

struct wl_egl_window;
struct wl_surface;

struct SWaylandEglWindowApi
{
    struct wl_egl_window* ( *window_create )( struct wl_surface* surface, int width, int height );
    void ( *window_destroy )( struct wl_egl_window* window );
    void ( *window_resize )( struct wl_egl_window* window, int width, int height, int dx, int dy );
    void ( *window_get_attached_size )( struct wl_egl_window* window, int* width, int* height );
};
typedef struct SWaylandEglWindowApi SWaylandEglWindowApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      FIXED POINT                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline double
gucef_wl_fixed_to_double( wl_fixed_t f )
{
    return f / 256.0;
}

static inline wl_fixed_t
gucef_wl_fixed_from_int( int i )
{
    return (wl_fixed_t) ( i * 256 );
}

static inline int
gucef_wl_fixed_to_int( wl_fixed_t f )
{
    return f / 256;
}

#ifdef __cplusplus
}
#endif

#endif /* GUCEF_PLUGINGLUE_WAYLAND_ABI_H ? */
