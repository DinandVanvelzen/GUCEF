/* Generated from the Wayland protocol XML files with a scanner that makes the output of wayland-scanner (public-code and
 * client-header) with request wrappers that call through a function table, so no libwayland-client is linked.
 * The copyright notices of the protocol files follow. Do not edit by hand. */

/* ---- wayland (wayland.xml) ----

    Copyright © 2008-2011 Kristian Høgsberg
    Copyright © 2010-2011 Intel Corporation
    Copyright © 2012-2013 Collabora, Ltd.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice (including the
    next paragraph) shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
  
 */

/* ---- xdg_shell (xdg-shell.xml) ----

    Copyright © 2008-2013 Kristian Høgsberg
    Copyright © 2013      Rafael Antognolli
    Copyright © 2013      Jasper St. Pierre
    Copyright © 2010-2013 Intel Corporation
    Copyright © 2015-2017 Samsung Electronics Co., Ltd
    Copyright © 2015-2017 Red Hat Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- viewporter (viewporter.xml) ----

    Copyright © 2013-2016 Collabora, Ltd.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- fractional_scale_v1 (fractional-scale-v1.xml) ----

    Copyright © 2022 Kenny Levinsen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_decoration_unstable_v1 (xdg-decoration-unstable-v1.xml) ----

    Copyright © 2018 Simon Ser

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- cursor_shape_v1 (cursor-shape-v1.xml) ----

    Copyright 2018 The Chromium Authors
    Copyright 2023 Simon Ser

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_output_unstable_v1 (xdg-output-unstable-v1.xml) ----

    Copyright © 2017 Red Hat Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_activation_v1 (xdg-activation-v1.xml) ----

    Copyright © 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright © 2020 Carlos Garnacho <carlosg@gnome.org>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H

#include "pluginglueWAYLAND_Abi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wl_buffer;
struct wl_callback;
struct wl_compositor;
struct wl_keyboard;
struct wl_output;
struct wl_pointer;
struct wl_region;
struct wl_registry;
struct wl_seat;
struct wl_shm;
struct wl_shm_pool;
struct wl_surface;
struct wl_touch;
struct wp_cursor_shape_device_v1;
struct wp_cursor_shape_manager_v1;
struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_v1;
struct wp_viewport;
struct wp_viewporter;
struct xdg_activation_token_v1;
struct xdg_activation_v1;
struct xdg_popup;
struct xdg_positioner;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_wm_base;
struct zwp_tablet_tool_v2;
struct zxdg_decoration_manager_v1;
struct zxdg_output_manager_v1;
struct zxdg_output_v1;
struct zxdg_toplevel_decoration_v1;

GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_buffer_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_callback_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_compositor_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_keyboard_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_output_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_pointer_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_region_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_registry_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_seat_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_shm_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_shm_pool_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_surface_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wl_touch_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_cursor_shape_device_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_cursor_shape_manager_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_fractional_scale_manager_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_fractional_scale_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_viewport_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_wp_viewporter_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_activation_token_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_activation_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_popup_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_positioner_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_surface_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_toplevel_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_xdg_wm_base_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_zxdg_decoration_manager_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_zxdg_output_manager_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_zxdg_output_v1_interface;
GUCEF_WL_PROTO_API const struct wl_interface gucef_zxdg_toplevel_decoration_v1_interface;

/* ---- wayland ---- */

#define GUCEF_WL_WL_REGISTRY_VERSION 1
#define GUCEF_WL_WL_REGISTRY_BIND_OPCODE 0
#define GUCEF_WL_WL_REGISTRY_BIND_SINCE_VERSION 1
#define GUCEF_WL_WL_REGISTRY_GLOBAL_EVENT_OPCODE 0
#define GUCEF_WL_WL_REGISTRY_GLOBAL_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_REGISTRY_GLOBAL_REMOVE_EVENT_OPCODE 1
#define GUCEF_WL_WL_REGISTRY_GLOBAL_REMOVE_EVENT_SINCE_VERSION 1
struct gucef_wl_registry_listener {
    void (*global)( void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version );
    void (*global_remove)( void* data, struct wl_registry* wl_registry, uint32_t name );
};
#define GUCEF_WL_WL_REGISTRY_EVENT_COUNT 2
typedef char gucef_wl_registry_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_registry_listener ) == GUCEF_WL_WL_REGISTRY_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_registry_add_listener( const SWaylandApi* api, struct wl_registry* proxy, const struct gucef_wl_registry_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_registry_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_registry" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void*
gucef_wl_registry_bind( const SWaylandApi* api, struct wl_registry* proxy, uint32_t name, const struct wl_interface* iface, uint32_t version )
{
    return (void*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, iface, version, 0, name, iface->name, version, NULL );
}
static inline void
gucef_wl_registry_destroy_proxy( const SWaylandApi* api, struct wl_registry* proxy )
{
    api->proxy_destroy( (struct wl_proxy*) proxy );
}

#define GUCEF_WL_WL_CALLBACK_VERSION 1
#define GUCEF_WL_WL_CALLBACK_DONE_EVENT_OPCODE 0
#define GUCEF_WL_WL_CALLBACK_DONE_EVENT_SINCE_VERSION 1
struct gucef_wl_callback_listener {
    void (*done)( void* data, struct wl_callback* wl_callback, uint32_t callback_data );
};
#define GUCEF_WL_WL_CALLBACK_EVENT_COUNT 1
typedef char gucef_wl_callback_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_callback_listener ) == GUCEF_WL_WL_CALLBACK_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_callback_add_listener( const SWaylandApi* api, struct wl_callback* proxy, const struct gucef_wl_callback_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_callback_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_callback" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_callback_destroy_proxy( const SWaylandApi* api, struct wl_callback* proxy )
{
    api->proxy_destroy( (struct wl_proxy*) proxy );
}

#define GUCEF_WL_WL_COMPOSITOR_VERSION 7
#define GUCEF_WL_WL_COMPOSITOR_CREATE_SURFACE_OPCODE 0
#define GUCEF_WL_WL_COMPOSITOR_CREATE_SURFACE_SINCE_VERSION 1
#define GUCEF_WL_WL_COMPOSITOR_CREATE_REGION_OPCODE 1
#define GUCEF_WL_WL_COMPOSITOR_CREATE_REGION_SINCE_VERSION 1
#define GUCEF_WL_WL_COMPOSITOR_RELEASE_OPCODE 2
#define GUCEF_WL_WL_COMPOSITOR_RELEASE_SINCE_VERSION 7
static inline struct wl_surface*
gucef_wl_compositor_create_surface( const SWaylandApi* api, struct wl_compositor* proxy )
{
    return (struct wl_surface*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, &gucef_wl_surface_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline struct wl_region*
gucef_wl_compositor_create_region( const SWaylandApi* api, struct wl_compositor* proxy )
{
    return (struct wl_region*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_wl_region_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline void
gucef_wl_compositor_release( const SWaylandApi* api, struct wl_compositor* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_SHM_POOL_VERSION 3
#define GUCEF_WL_WL_SHM_POOL_ERROR_INVALID_FORMAT 0
#define GUCEF_WL_WL_SHM_POOL_ERROR_INVALID_STRIDE 1
#define GUCEF_WL_WL_SHM_POOL_CREATE_BUFFER_OPCODE 0
#define GUCEF_WL_WL_SHM_POOL_CREATE_BUFFER_SINCE_VERSION 1
#define GUCEF_WL_WL_SHM_POOL_DESTROY_OPCODE 1
#define GUCEF_WL_WL_SHM_POOL_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WL_SHM_POOL_RESIZE_OPCODE 2
#define GUCEF_WL_WL_SHM_POOL_RESIZE_SINCE_VERSION 1
static inline struct wl_buffer*
gucef_wl_shm_pool_create_buffer( const SWaylandApi* api, struct wl_shm_pool* proxy, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format )
{
    return (struct wl_buffer*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, &gucef_wl_buffer_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, offset, width, height, stride, format );
}
static inline void
gucef_wl_shm_pool_destroy( const SWaylandApi* api, struct wl_shm_pool* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_wl_shm_pool_resize( const SWaylandApi* api, struct wl_shm_pool* proxy, int32_t size )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, size );
}

#define GUCEF_WL_WL_SHM_VERSION 3
#define GUCEF_WL_WL_SHM_ERROR_INVALID_FORMAT 0
#define GUCEF_WL_WL_SHM_ERROR_INVALID_STRIDE 1
#define GUCEF_WL_WL_SHM_ERROR_INVALID_FD 2
#define GUCEF_WL_WL_SHM_FORMAT_ARGB8888 0
#define GUCEF_WL_WL_SHM_FORMAT_XRGB8888 1
#define GUCEF_WL_WL_SHM_FORMAT_C8 538982467
#define GUCEF_WL_WL_SHM_FORMAT_RGB332 943867730
#define GUCEF_WL_WL_SHM_FORMAT_BGR233 944916290
#define GUCEF_WL_WL_SHM_FORMAT_XRGB4444 842093144
#define GUCEF_WL_WL_SHM_FORMAT_XBGR4444 842089048
#define GUCEF_WL_WL_SHM_FORMAT_RGBX4444 842094674
#define GUCEF_WL_WL_SHM_FORMAT_BGRX4444 842094658
#define GUCEF_WL_WL_SHM_FORMAT_ARGB4444 842093121
#define GUCEF_WL_WL_SHM_FORMAT_ABGR4444 842089025
#define GUCEF_WL_WL_SHM_FORMAT_RGBA4444 842088786
#define GUCEF_WL_WL_SHM_FORMAT_BGRA4444 842088770
#define GUCEF_WL_WL_SHM_FORMAT_XRGB1555 892424792
#define GUCEF_WL_WL_SHM_FORMAT_XBGR1555 892420696
#define GUCEF_WL_WL_SHM_FORMAT_RGBX5551 892426322
#define GUCEF_WL_WL_SHM_FORMAT_BGRX5551 892426306
#define GUCEF_WL_WL_SHM_FORMAT_ARGB1555 892424769
#define GUCEF_WL_WL_SHM_FORMAT_ABGR1555 892420673
#define GUCEF_WL_WL_SHM_FORMAT_RGBA5551 892420434
#define GUCEF_WL_WL_SHM_FORMAT_BGRA5551 892420418
#define GUCEF_WL_WL_SHM_FORMAT_RGB565 909199186
#define GUCEF_WL_WL_SHM_FORMAT_BGR565 909199170
#define GUCEF_WL_WL_SHM_FORMAT_RGB888 875710290
#define GUCEF_WL_WL_SHM_FORMAT_BGR888 875710274
#define GUCEF_WL_WL_SHM_FORMAT_XBGR8888 875709016
#define GUCEF_WL_WL_SHM_FORMAT_RGBX8888 875714642
#define GUCEF_WL_WL_SHM_FORMAT_BGRX8888 875714626
#define GUCEF_WL_WL_SHM_FORMAT_ABGR8888 875708993
#define GUCEF_WL_WL_SHM_FORMAT_RGBA8888 875708754
#define GUCEF_WL_WL_SHM_FORMAT_BGRA8888 875708738
#define GUCEF_WL_WL_SHM_FORMAT_XRGB2101010 808669784
#define GUCEF_WL_WL_SHM_FORMAT_XBGR2101010 808665688
#define GUCEF_WL_WL_SHM_FORMAT_RGBX1010102 808671314
#define GUCEF_WL_WL_SHM_FORMAT_BGRX1010102 808671298
#define GUCEF_WL_WL_SHM_FORMAT_ARGB2101010 808669761
#define GUCEF_WL_WL_SHM_FORMAT_ABGR2101010 808665665
#define GUCEF_WL_WL_SHM_FORMAT_RGBA1010102 808665426
#define GUCEF_WL_WL_SHM_FORMAT_BGRA1010102 808665410
#define GUCEF_WL_WL_SHM_FORMAT_YUYV 1448695129
#define GUCEF_WL_WL_SHM_FORMAT_YVYU 1431918169
#define GUCEF_WL_WL_SHM_FORMAT_UYVY 1498831189
#define GUCEF_WL_WL_SHM_FORMAT_VYUY 1498765654
#define GUCEF_WL_WL_SHM_FORMAT_AYUV 1448433985
#define GUCEF_WL_WL_SHM_FORMAT_NV12 842094158
#define GUCEF_WL_WL_SHM_FORMAT_NV21 825382478
#define GUCEF_WL_WL_SHM_FORMAT_NV16 909203022
#define GUCEF_WL_WL_SHM_FORMAT_NV61 825644622
#define GUCEF_WL_WL_SHM_FORMAT_YUV410 961959257
#define GUCEF_WL_WL_SHM_FORMAT_YVU410 961893977
#define GUCEF_WL_WL_SHM_FORMAT_YUV411 825316697
#define GUCEF_WL_WL_SHM_FORMAT_YVU411 825316953
#define GUCEF_WL_WL_SHM_FORMAT_YUV420 842093913
#define GUCEF_WL_WL_SHM_FORMAT_YVU420 842094169
#define GUCEF_WL_WL_SHM_FORMAT_YUV422 909202777
#define GUCEF_WL_WL_SHM_FORMAT_YVU422 909203033
#define GUCEF_WL_WL_SHM_FORMAT_YUV444 875713881
#define GUCEF_WL_WL_SHM_FORMAT_YVU444 875714137
#define GUCEF_WL_WL_SHM_FORMAT_R8 538982482
#define GUCEF_WL_WL_SHM_FORMAT_R16 540422482
#define GUCEF_WL_WL_SHM_FORMAT_RG88 943212370
#define GUCEF_WL_WL_SHM_FORMAT_GR88 943215175
#define GUCEF_WL_WL_SHM_FORMAT_RG1616 842221394
#define GUCEF_WL_WL_SHM_FORMAT_GR1616 842224199
#define GUCEF_WL_WL_SHM_FORMAT_XRGB16161616F 1211388504
#define GUCEF_WL_WL_SHM_FORMAT_XBGR16161616F 1211384408
#define GUCEF_WL_WL_SHM_FORMAT_ARGB16161616F 1211388481
#define GUCEF_WL_WL_SHM_FORMAT_ABGR16161616F 1211384385
#define GUCEF_WL_WL_SHM_FORMAT_XYUV8888 1448434008
#define GUCEF_WL_WL_SHM_FORMAT_VUY888 875713878
#define GUCEF_WL_WL_SHM_FORMAT_VUY101010 808670550
#define GUCEF_WL_WL_SHM_FORMAT_Y210 808530521
#define GUCEF_WL_WL_SHM_FORMAT_Y212 842084953
#define GUCEF_WL_WL_SHM_FORMAT_Y216 909193817
#define GUCEF_WL_WL_SHM_FORMAT_Y410 808531033
#define GUCEF_WL_WL_SHM_FORMAT_Y412 842085465
#define GUCEF_WL_WL_SHM_FORMAT_Y416 909194329
#define GUCEF_WL_WL_SHM_FORMAT_XVYU2101010 808670808
#define GUCEF_WL_WL_SHM_FORMAT_XVYU12_16161616 909334104
#define GUCEF_WL_WL_SHM_FORMAT_XVYU16161616 942954072
#define GUCEF_WL_WL_SHM_FORMAT_Y0L0 810299481
#define GUCEF_WL_WL_SHM_FORMAT_X0L0 810299480
#define GUCEF_WL_WL_SHM_FORMAT_Y0L2 843853913
#define GUCEF_WL_WL_SHM_FORMAT_X0L2 843853912
#define GUCEF_WL_WL_SHM_FORMAT_YUV420_8BIT 942691673
#define GUCEF_WL_WL_SHM_FORMAT_YUV420_10BIT 808539481
#define GUCEF_WL_WL_SHM_FORMAT_XRGB8888_A8 943805016
#define GUCEF_WL_WL_SHM_FORMAT_XBGR8888_A8 943800920
#define GUCEF_WL_WL_SHM_FORMAT_RGBX8888_A8 943806546
#define GUCEF_WL_WL_SHM_FORMAT_BGRX8888_A8 943806530
#define GUCEF_WL_WL_SHM_FORMAT_RGB888_A8 943798354
#define GUCEF_WL_WL_SHM_FORMAT_BGR888_A8 943798338
#define GUCEF_WL_WL_SHM_FORMAT_RGB565_A8 943797586
#define GUCEF_WL_WL_SHM_FORMAT_BGR565_A8 943797570
#define GUCEF_WL_WL_SHM_FORMAT_NV24 875714126
#define GUCEF_WL_WL_SHM_FORMAT_NV42 842290766
#define GUCEF_WL_WL_SHM_FORMAT_P210 808530512
#define GUCEF_WL_WL_SHM_FORMAT_P010 808530000
#define GUCEF_WL_WL_SHM_FORMAT_P012 842084432
#define GUCEF_WL_WL_SHM_FORMAT_P016 909193296
#define GUCEF_WL_WL_SHM_FORMAT_AXBXGXRX106106106106 808534593
#define GUCEF_WL_WL_SHM_FORMAT_NV15 892425806
#define GUCEF_WL_WL_SHM_FORMAT_Q410 808531025
#define GUCEF_WL_WL_SHM_FORMAT_Q401 825242705
#define GUCEF_WL_WL_SHM_FORMAT_XRGB16161616 942953048
#define GUCEF_WL_WL_SHM_FORMAT_XBGR16161616 942948952
#define GUCEF_WL_WL_SHM_FORMAT_ARGB16161616 942953025
#define GUCEF_WL_WL_SHM_FORMAT_ABGR16161616 942948929
#define GUCEF_WL_WL_SHM_FORMAT_C1 538980675
#define GUCEF_WL_WL_SHM_FORMAT_C2 538980931
#define GUCEF_WL_WL_SHM_FORMAT_C4 538981443
#define GUCEF_WL_WL_SHM_FORMAT_D1 538980676
#define GUCEF_WL_WL_SHM_FORMAT_D2 538980932
#define GUCEF_WL_WL_SHM_FORMAT_D4 538981444
#define GUCEF_WL_WL_SHM_FORMAT_D8 538982468
#define GUCEF_WL_WL_SHM_FORMAT_R1 538980690
#define GUCEF_WL_WL_SHM_FORMAT_R2 538980946
#define GUCEF_WL_WL_SHM_FORMAT_R4 538981458
#define GUCEF_WL_WL_SHM_FORMAT_R10 540029266
#define GUCEF_WL_WL_SHM_FORMAT_R12 540160338
#define GUCEF_WL_WL_SHM_FORMAT_AVUY8888 1498764865
#define GUCEF_WL_WL_SHM_FORMAT_XVUY8888 1498764888
#define GUCEF_WL_WL_SHM_FORMAT_P030 808661072
#define GUCEF_WL_WL_SHM_FORMAT_RGB161616 942950226
#define GUCEF_WL_WL_SHM_FORMAT_BGR161616 942950210
#define GUCEF_WL_WL_SHM_FORMAT_R16F 1210064978
#define GUCEF_WL_WL_SHM_FORMAT_GR1616F 1210077767
#define GUCEF_WL_WL_SHM_FORMAT_BGR161616F 1213351746
#define GUCEF_WL_WL_SHM_FORMAT_R32F 1176510546
#define GUCEF_WL_WL_SHM_FORMAT_GR3232F 1176523335
#define GUCEF_WL_WL_SHM_FORMAT_BGR323232F 1179797314
#define GUCEF_WL_WL_SHM_FORMAT_ABGR32323232F 1178092097
#define GUCEF_WL_WL_SHM_FORMAT_NV20 808605262
#define GUCEF_WL_WL_SHM_FORMAT_NV30 808670798
#define GUCEF_WL_WL_SHM_FORMAT_S010 808530003
#define GUCEF_WL_WL_SHM_FORMAT_S210 808530515
#define GUCEF_WL_WL_SHM_FORMAT_S410 808531027
#define GUCEF_WL_WL_SHM_FORMAT_S012 842084435
#define GUCEF_WL_WL_SHM_FORMAT_S212 842084947
#define GUCEF_WL_WL_SHM_FORMAT_S412 842085459
#define GUCEF_WL_WL_SHM_FORMAT_S016 909193299
#define GUCEF_WL_WL_SHM_FORMAT_S216 909193811
#define GUCEF_WL_WL_SHM_FORMAT_S416 909194323
#define GUCEF_WL_WL_SHM_FORMAT_XVUY2101010 808671576
#define GUCEF_WL_WL_SHM_FORMAT_P230 808661584
#define GUCEF_WL_WL_SHM_FORMAT_T430 808662100
#define GUCEF_WL_WL_SHM_FORMAT_Y8 1497715271
#define GUCEF_WL_WL_SHM_FORMAT_XYYY2101010 876695641
#define GUCEF_WL_WL_SHM_CREATE_POOL_OPCODE 0
#define GUCEF_WL_WL_SHM_CREATE_POOL_SINCE_VERSION 1
#define GUCEF_WL_WL_SHM_RELEASE_OPCODE 1
#define GUCEF_WL_WL_SHM_RELEASE_SINCE_VERSION 2
#define GUCEF_WL_WL_SHM_FORMAT_EVENT_OPCODE 0
#define GUCEF_WL_WL_SHM_FORMAT_EVENT_SINCE_VERSION 1
struct gucef_wl_shm_listener {
    void (*format)( void* data, struct wl_shm* wl_shm, uint32_t format );
};
#define GUCEF_WL_WL_SHM_EVENT_COUNT 1
typedef char gucef_wl_shm_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_shm_listener ) == GUCEF_WL_WL_SHM_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_shm_add_listener( const SWaylandApi* api, struct wl_shm* proxy, const struct gucef_wl_shm_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_shm_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_shm" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline struct wl_shm_pool*
gucef_wl_shm_create_pool( const SWaylandApi* api, struct wl_shm* proxy, int32_t fd, int32_t size )
{
    return (struct wl_shm_pool*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, &gucef_wl_shm_pool_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, fd, size );
}
static inline void
gucef_wl_shm_release( const SWaylandApi* api, struct wl_shm* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_BUFFER_VERSION 1
#define GUCEF_WL_WL_BUFFER_DESTROY_OPCODE 0
#define GUCEF_WL_WL_BUFFER_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WL_BUFFER_RELEASE_EVENT_OPCODE 0
#define GUCEF_WL_WL_BUFFER_RELEASE_EVENT_SINCE_VERSION 1
struct gucef_wl_buffer_listener {
    void (*release)( void* data, struct wl_buffer* wl_buffer );
};
#define GUCEF_WL_WL_BUFFER_EVENT_COUNT 1
typedef char gucef_wl_buffer_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_buffer_listener ) == GUCEF_WL_WL_BUFFER_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_buffer_add_listener( const SWaylandApi* api, struct wl_buffer* proxy, const struct gucef_wl_buffer_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_buffer_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_buffer" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_buffer_destroy( const SWaylandApi* api, struct wl_buffer* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_SURFACE_VERSION 7
#define GUCEF_WL_WL_SURFACE_ERROR_INVALID_SCALE 0
#define GUCEF_WL_WL_SURFACE_ERROR_INVALID_TRANSFORM 1
#define GUCEF_WL_WL_SURFACE_ERROR_INVALID_SIZE 2
#define GUCEF_WL_WL_SURFACE_ERROR_INVALID_OFFSET 3
#define GUCEF_WL_WL_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT 4
#define GUCEF_WL_WL_SURFACE_ERROR_NO_BUFFER 5
#define GUCEF_WL_WL_SURFACE_DESTROY_OPCODE 0
#define GUCEF_WL_WL_SURFACE_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_ATTACH_OPCODE 1
#define GUCEF_WL_WL_SURFACE_ATTACH_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_DAMAGE_OPCODE 2
#define GUCEF_WL_WL_SURFACE_DAMAGE_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_FRAME_OPCODE 3
#define GUCEF_WL_WL_SURFACE_FRAME_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_SET_OPAQUE_REGION_OPCODE 4
#define GUCEF_WL_WL_SURFACE_SET_OPAQUE_REGION_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_SET_INPUT_REGION_OPCODE 5
#define GUCEF_WL_WL_SURFACE_SET_INPUT_REGION_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_COMMIT_OPCODE 6
#define GUCEF_WL_WL_SURFACE_COMMIT_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_SET_BUFFER_TRANSFORM_OPCODE 7
#define GUCEF_WL_WL_SURFACE_SET_BUFFER_TRANSFORM_SINCE_VERSION 2
#define GUCEF_WL_WL_SURFACE_SET_BUFFER_SCALE_OPCODE 8
#define GUCEF_WL_WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION 3
#define GUCEF_WL_WL_SURFACE_DAMAGE_BUFFER_OPCODE 9
#define GUCEF_WL_WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION 4
#define GUCEF_WL_WL_SURFACE_OFFSET_OPCODE 10
#define GUCEF_WL_WL_SURFACE_OFFSET_SINCE_VERSION 5
#define GUCEF_WL_WL_SURFACE_GET_RELEASE_OPCODE 11
#define GUCEF_WL_WL_SURFACE_GET_RELEASE_SINCE_VERSION 7
#define GUCEF_WL_WL_SURFACE_ENTER_EVENT_OPCODE 0
#define GUCEF_WL_WL_SURFACE_ENTER_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_LEAVE_EVENT_OPCODE 1
#define GUCEF_WL_WL_SURFACE_LEAVE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_SURFACE_PREFERRED_BUFFER_SCALE_EVENT_OPCODE 2
#define GUCEF_WL_WL_SURFACE_PREFERRED_BUFFER_SCALE_EVENT_SINCE_VERSION 6
#define GUCEF_WL_WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_EVENT_OPCODE 3
#define GUCEF_WL_WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_EVENT_SINCE_VERSION 6
struct gucef_wl_surface_listener {
    void (*enter)( void* data, struct wl_surface* wl_surface, struct wl_output* output );
    void (*leave)( void* data, struct wl_surface* wl_surface, struct wl_output* output );
    void (*preferred_buffer_scale)( void* data, struct wl_surface* wl_surface, int32_t factor );
    void (*preferred_buffer_transform)( void* data, struct wl_surface* wl_surface, uint32_t transform );
};
#define GUCEF_WL_WL_SURFACE_EVENT_COUNT 4
typedef char gucef_wl_surface_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_surface_listener ) == GUCEF_WL_WL_SURFACE_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_surface_add_listener( const SWaylandApi* api, struct wl_surface* proxy, const struct gucef_wl_surface_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_surface_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_surface" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_surface_destroy( const SWaylandApi* api, struct wl_surface* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_wl_surface_attach( const SWaylandApi* api, struct wl_surface* proxy, struct wl_buffer* buffer, int32_t x, int32_t y )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, buffer, x, y );
}
static inline void
gucef_wl_surface_damage( const SWaylandApi* api, struct wl_surface* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline struct wl_callback*
gucef_wl_surface_frame( const SWaylandApi* api, struct wl_surface* proxy )
{
    return (struct wl_callback*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, &gucef_wl_callback_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline void
gucef_wl_surface_set_opaque_region( const SWaylandApi* api, struct wl_surface* proxy, struct wl_region* region )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 4, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, region );
}
static inline void
gucef_wl_surface_set_input_region( const SWaylandApi* api, struct wl_surface* proxy, struct wl_region* region )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 5, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, region );
}
static inline void
gucef_wl_surface_commit( const SWaylandApi* api, struct wl_surface* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 6, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_wl_surface_set_buffer_transform( const SWaylandApi* api, struct wl_surface* proxy, int32_t transform )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 7, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, transform );
}
static inline void
gucef_wl_surface_set_buffer_scale( const SWaylandApi* api, struct wl_surface* proxy, int32_t scale )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 8, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, scale );
}
static inline void
gucef_wl_surface_damage_buffer( const SWaylandApi* api, struct wl_surface* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 9, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline void
gucef_wl_surface_offset( const SWaylandApi* api, struct wl_surface* proxy, int32_t x, int32_t y )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 10, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y );
}
static inline struct wl_callback*
gucef_wl_surface_get_release( const SWaylandApi* api, struct wl_surface* proxy )
{
    return (struct wl_callback*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 11, &gucef_wl_callback_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}

#define GUCEF_WL_WL_SEAT_VERSION 11
#define GUCEF_WL_WL_SEAT_CAPABILITY_POINTER 1
#define GUCEF_WL_WL_SEAT_CAPABILITY_KEYBOARD 2
#define GUCEF_WL_WL_SEAT_CAPABILITY_TOUCH 4
#define GUCEF_WL_WL_SEAT_ERROR_MISSING_CAPABILITY 0
#define GUCEF_WL_WL_SEAT_GET_POINTER_OPCODE 0
#define GUCEF_WL_WL_SEAT_GET_POINTER_SINCE_VERSION 1
#define GUCEF_WL_WL_SEAT_GET_KEYBOARD_OPCODE 1
#define GUCEF_WL_WL_SEAT_GET_KEYBOARD_SINCE_VERSION 1
#define GUCEF_WL_WL_SEAT_GET_TOUCH_OPCODE 2
#define GUCEF_WL_WL_SEAT_GET_TOUCH_SINCE_VERSION 1
#define GUCEF_WL_WL_SEAT_RELEASE_OPCODE 3
#define GUCEF_WL_WL_SEAT_RELEASE_SINCE_VERSION 5
#define GUCEF_WL_WL_SEAT_CAPABILITIES_EVENT_OPCODE 0
#define GUCEF_WL_WL_SEAT_CAPABILITIES_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_SEAT_NAME_EVENT_OPCODE 1
#define GUCEF_WL_WL_SEAT_NAME_EVENT_SINCE_VERSION 2
struct gucef_wl_seat_listener {
    void (*capabilities)( void* data, struct wl_seat* wl_seat, uint32_t capabilities );
    void (*name)( void* data, struct wl_seat* wl_seat, const char* name );
};
#define GUCEF_WL_WL_SEAT_EVENT_COUNT 2
typedef char gucef_wl_seat_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_seat_listener ) == GUCEF_WL_WL_SEAT_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_seat_add_listener( const SWaylandApi* api, struct wl_seat* proxy, const struct gucef_wl_seat_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_seat_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_seat" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline struct wl_pointer*
gucef_wl_seat_get_pointer( const SWaylandApi* api, struct wl_seat* proxy )
{
    return (struct wl_pointer*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, &gucef_wl_pointer_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline struct wl_keyboard*
gucef_wl_seat_get_keyboard( const SWaylandApi* api, struct wl_seat* proxy )
{
    return (struct wl_keyboard*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_wl_keyboard_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline struct wl_touch*
gucef_wl_seat_get_touch( const SWaylandApi* api, struct wl_seat* proxy )
{
    return (struct wl_touch*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, &gucef_wl_touch_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline void
gucef_wl_seat_release( const SWaylandApi* api, struct wl_seat* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_POINTER_VERSION 11
#define GUCEF_WL_WL_POINTER_ERROR_ROLE 0
#define GUCEF_WL_WL_POINTER_BUTTON_STATE_RELEASED 0
#define GUCEF_WL_WL_POINTER_BUTTON_STATE_PRESSED 1
#define GUCEF_WL_WL_POINTER_AXIS_VERTICAL_SCROLL 0
#define GUCEF_WL_WL_POINTER_AXIS_HORIZONTAL_SCROLL 1
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL 0
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_FINGER 1
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_CONTINUOUS 2
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL_TILT 3
#define GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL 0
#define GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED 1
#define GUCEF_WL_WL_POINTER_SET_CURSOR_OPCODE 0
#define GUCEF_WL_WL_POINTER_SET_CURSOR_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_RELEASE_OPCODE 1
#define GUCEF_WL_WL_POINTER_RELEASE_SINCE_VERSION 3
#define GUCEF_WL_WL_POINTER_ENTER_EVENT_OPCODE 0
#define GUCEF_WL_WL_POINTER_ENTER_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_LEAVE_EVENT_OPCODE 1
#define GUCEF_WL_WL_POINTER_LEAVE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_MOTION_EVENT_OPCODE 2
#define GUCEF_WL_WL_POINTER_MOTION_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_BUTTON_EVENT_OPCODE 3
#define GUCEF_WL_WL_POINTER_BUTTON_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_AXIS_EVENT_OPCODE 4
#define GUCEF_WL_WL_POINTER_AXIS_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_POINTER_FRAME_EVENT_OPCODE 5
#define GUCEF_WL_WL_POINTER_FRAME_EVENT_SINCE_VERSION 5
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_EVENT_OPCODE 6
#define GUCEF_WL_WL_POINTER_AXIS_SOURCE_EVENT_SINCE_VERSION 5
#define GUCEF_WL_WL_POINTER_AXIS_STOP_EVENT_OPCODE 7
#define GUCEF_WL_WL_POINTER_AXIS_STOP_EVENT_SINCE_VERSION 5
#define GUCEF_WL_WL_POINTER_AXIS_DISCRETE_EVENT_OPCODE 8
#define GUCEF_WL_WL_POINTER_AXIS_DISCRETE_EVENT_SINCE_VERSION 5
#define GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_OPCODE 9
#define GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_SINCE_VERSION 8
#define GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_OPCODE 10
#define GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_SINCE_VERSION 9
#define GUCEF_WL_WL_POINTER_WARP_EVENT_OPCODE 11
#define GUCEF_WL_WL_POINTER_WARP_EVENT_SINCE_VERSION 11
struct gucef_wl_pointer_listener {
    void (*enter)( void* data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y );
    void (*leave)( void* data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface );
    void (*motion)( void* data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y );
    void (*button)( void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state );
    void (*axis)( void* data, struct wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value );
    void (*frame)( void* data, struct wl_pointer* wl_pointer );
    void (*axis_source)( void* data, struct wl_pointer* wl_pointer, uint32_t axis_source );
    void (*axis_stop)( void* data, struct wl_pointer* wl_pointer, uint32_t time, uint32_t axis );
    void (*axis_discrete)( void* data, struct wl_pointer* wl_pointer, uint32_t axis, int32_t discrete );
    void (*axis_value120)( void* data, struct wl_pointer* wl_pointer, uint32_t axis, int32_t value120 );
    void (*axis_relative_direction)( void* data, struct wl_pointer* wl_pointer, uint32_t axis, uint32_t direction );
    void (*warp)( void* data, struct wl_pointer* wl_pointer, wl_fixed_t surface_x, wl_fixed_t surface_y );
};
#define GUCEF_WL_WL_POINTER_EVENT_COUNT 12
typedef char gucef_wl_pointer_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_pointer_listener ) == GUCEF_WL_WL_POINTER_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_pointer_add_listener( const SWaylandApi* api, struct wl_pointer* proxy, const struct gucef_wl_pointer_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_pointer_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_pointer" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_pointer_set_cursor( const SWaylandApi* api, struct wl_pointer* proxy, uint32_t serial, struct wl_surface* surface, int32_t hotspot_x, int32_t hotspot_y )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial, surface, hotspot_x, hotspot_y );
}
static inline void
gucef_wl_pointer_release( const SWaylandApi* api, struct wl_pointer* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_KEYBOARD_VERSION 11
#define GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP 0
#define GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 1
#define GUCEF_WL_WL_KEYBOARD_KEY_STATE_RELEASED 0
#define GUCEF_WL_WL_KEYBOARD_KEY_STATE_PRESSED 1
#define GUCEF_WL_WL_KEYBOARD_KEY_STATE_REPEATED 2
#define GUCEF_WL_WL_KEYBOARD_RELEASE_OPCODE 0
#define GUCEF_WL_WL_KEYBOARD_RELEASE_SINCE_VERSION 3
#define GUCEF_WL_WL_KEYBOARD_KEYMAP_EVENT_OPCODE 0
#define GUCEF_WL_WL_KEYBOARD_KEYMAP_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_KEYBOARD_ENTER_EVENT_OPCODE 1
#define GUCEF_WL_WL_KEYBOARD_ENTER_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_KEYBOARD_LEAVE_EVENT_OPCODE 2
#define GUCEF_WL_WL_KEYBOARD_LEAVE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_KEYBOARD_KEY_EVENT_OPCODE 3
#define GUCEF_WL_WL_KEYBOARD_KEY_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_KEYBOARD_MODIFIERS_EVENT_OPCODE 4
#define GUCEF_WL_WL_KEYBOARD_MODIFIERS_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_KEYBOARD_REPEAT_INFO_EVENT_OPCODE 5
#define GUCEF_WL_WL_KEYBOARD_REPEAT_INFO_EVENT_SINCE_VERSION 4
struct gucef_wl_keyboard_listener {
    void (*keymap)( void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size );
    void (*enter)( void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys );
    void (*leave)( void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface );
    void (*key)( void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state );
    void (*modifiers)( void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group );
    void (*repeat_info)( void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay );
};
#define GUCEF_WL_WL_KEYBOARD_EVENT_COUNT 6
typedef char gucef_wl_keyboard_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_keyboard_listener ) == GUCEF_WL_WL_KEYBOARD_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_keyboard_add_listener( const SWaylandApi* api, struct wl_keyboard* proxy, const struct gucef_wl_keyboard_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_keyboard_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_keyboard" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_keyboard_release( const SWaylandApi* api, struct wl_keyboard* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_TOUCH_VERSION 11
#define GUCEF_WL_WL_TOUCH_RELEASE_OPCODE 0
#define GUCEF_WL_WL_TOUCH_RELEASE_SINCE_VERSION 3
#define GUCEF_WL_WL_TOUCH_DOWN_EVENT_OPCODE 0
#define GUCEF_WL_WL_TOUCH_DOWN_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_TOUCH_UP_EVENT_OPCODE 1
#define GUCEF_WL_WL_TOUCH_UP_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_TOUCH_MOTION_EVENT_OPCODE 2
#define GUCEF_WL_WL_TOUCH_MOTION_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_TOUCH_FRAME_EVENT_OPCODE 3
#define GUCEF_WL_WL_TOUCH_FRAME_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_TOUCH_CANCEL_EVENT_OPCODE 4
#define GUCEF_WL_WL_TOUCH_CANCEL_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_TOUCH_SHAPE_EVENT_OPCODE 5
#define GUCEF_WL_WL_TOUCH_SHAPE_EVENT_SINCE_VERSION 6
#define GUCEF_WL_WL_TOUCH_ORIENTATION_EVENT_OPCODE 6
#define GUCEF_WL_WL_TOUCH_ORIENTATION_EVENT_SINCE_VERSION 6
struct gucef_wl_touch_listener {
    void (*down)( void* data, struct wl_touch* wl_touch, uint32_t serial, uint32_t time, struct wl_surface* surface, int32_t id, wl_fixed_t x, wl_fixed_t y );
    void (*up)( void* data, struct wl_touch* wl_touch, uint32_t serial, uint32_t time, int32_t id );
    void (*motion)( void* data, struct wl_touch* wl_touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y );
    void (*frame)( void* data, struct wl_touch* wl_touch );
    void (*cancel)( void* data, struct wl_touch* wl_touch );
    void (*shape)( void* data, struct wl_touch* wl_touch, int32_t id, wl_fixed_t major, wl_fixed_t minor );
    void (*orientation)( void* data, struct wl_touch* wl_touch, int32_t id, wl_fixed_t orientation );
};
#define GUCEF_WL_WL_TOUCH_EVENT_COUNT 7
typedef char gucef_wl_touch_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_touch_listener ) == GUCEF_WL_WL_TOUCH_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_touch_add_listener( const SWaylandApi* api, struct wl_touch* proxy, const struct gucef_wl_touch_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_touch_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_touch" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_touch_release( const SWaylandApi* api, struct wl_touch* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_OUTPUT_VERSION 4
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_UNKNOWN 0
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_NONE 1
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB 2
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR 3
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_VERTICAL_RGB 4
#define GUCEF_WL_WL_OUTPUT_SUBPIXEL_VERTICAL_BGR 5
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_NORMAL 0
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_90 1
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_180 2
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_270 3
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_FLIPPED 4
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_FLIPPED_90 5
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_FLIPPED_180 6
#define GUCEF_WL_WL_OUTPUT_TRANSFORM_FLIPPED_270 7
#define GUCEF_WL_WL_OUTPUT_MODE_CURRENT 1
#define GUCEF_WL_WL_OUTPUT_MODE_PREFERRED 2
#define GUCEF_WL_WL_OUTPUT_RELEASE_OPCODE 0
#define GUCEF_WL_WL_OUTPUT_RELEASE_SINCE_VERSION 3
#define GUCEF_WL_WL_OUTPUT_GEOMETRY_EVENT_OPCODE 0
#define GUCEF_WL_WL_OUTPUT_GEOMETRY_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_OUTPUT_MODE_EVENT_OPCODE 1
#define GUCEF_WL_WL_OUTPUT_MODE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_WL_OUTPUT_DONE_EVENT_OPCODE 2
#define GUCEF_WL_WL_OUTPUT_DONE_EVENT_SINCE_VERSION 2
#define GUCEF_WL_WL_OUTPUT_SCALE_EVENT_OPCODE 3
#define GUCEF_WL_WL_OUTPUT_SCALE_EVENT_SINCE_VERSION 2
#define GUCEF_WL_WL_OUTPUT_NAME_EVENT_OPCODE 4
#define GUCEF_WL_WL_OUTPUT_NAME_EVENT_SINCE_VERSION 4
#define GUCEF_WL_WL_OUTPUT_DESCRIPTION_EVENT_OPCODE 5
#define GUCEF_WL_WL_OUTPUT_DESCRIPTION_EVENT_SINCE_VERSION 4
struct gucef_wl_output_listener {
    void (*geometry)( void* data, struct wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform );
    void (*mode)( void* data, struct wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh );
    void (*done)( void* data, struct wl_output* wl_output );
    void (*scale)( void* data, struct wl_output* wl_output, int32_t factor );
    void (*name)( void* data, struct wl_output* wl_output, const char* name );
    void (*description)( void* data, struct wl_output* wl_output, const char* description );
};
#define GUCEF_WL_WL_OUTPUT_EVENT_COUNT 6
typedef char gucef_wl_output_listener_has_a_slot_per_event[ sizeof( struct gucef_wl_output_listener ) == GUCEF_WL_WL_OUTPUT_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wl_output_add_listener( const SWaylandApi* api, struct wl_output* proxy, const struct gucef_wl_output_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wl_output_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wl_output" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wl_output_release( const SWaylandApi* api, struct wl_output* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#define GUCEF_WL_WL_REGION_VERSION 7
#define GUCEF_WL_WL_REGION_DESTROY_OPCODE 0
#define GUCEF_WL_WL_REGION_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WL_REGION_ADD_OPCODE 1
#define GUCEF_WL_WL_REGION_ADD_SINCE_VERSION 1
#define GUCEF_WL_WL_REGION_SUBTRACT_OPCODE 2
#define GUCEF_WL_WL_REGION_SUBTRACT_SINCE_VERSION 1
static inline void
gucef_wl_region_destroy( const SWaylandApi* api, struct wl_region* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_wl_region_add( const SWaylandApi* api, struct wl_region* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline void
gucef_wl_region_subtract( const SWaylandApi* api, struct wl_region* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}

/* ---- xdg_shell ---- */

#define GUCEF_WL_XDG_WM_BASE_VERSION 7
#define GUCEF_WL_XDG_WM_BASE_ERROR_ROLE 0
#define GUCEF_WL_XDG_WM_BASE_ERROR_DEFUNCT_SURFACES 1
#define GUCEF_WL_XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP 2
#define GUCEF_WL_XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT 3
#define GUCEF_WL_XDG_WM_BASE_ERROR_INVALID_SURFACE_STATE 4
#define GUCEF_WL_XDG_WM_BASE_ERROR_INVALID_POSITIONER 5
#define GUCEF_WL_XDG_WM_BASE_ERROR_UNRESPONSIVE 6
#define GUCEF_WL_XDG_WM_BASE_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_WM_BASE_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_WM_BASE_CREATE_POSITIONER_OPCODE 1
#define GUCEF_WL_XDG_WM_BASE_CREATE_POSITIONER_SINCE_VERSION 1
#define GUCEF_WL_XDG_WM_BASE_GET_XDG_SURFACE_OPCODE 2
#define GUCEF_WL_XDG_WM_BASE_GET_XDG_SURFACE_SINCE_VERSION 1
#define GUCEF_WL_XDG_WM_BASE_PONG_OPCODE 3
#define GUCEF_WL_XDG_WM_BASE_PONG_SINCE_VERSION 1
#define GUCEF_WL_XDG_WM_BASE_PING_EVENT_OPCODE 0
#define GUCEF_WL_XDG_WM_BASE_PING_EVENT_SINCE_VERSION 1
struct gucef_xdg_wm_base_listener {
    void (*ping)( void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial );
};
#define GUCEF_WL_XDG_WM_BASE_EVENT_COUNT 1
typedef char gucef_xdg_wm_base_listener_has_a_slot_per_event[ sizeof( struct gucef_xdg_wm_base_listener ) == GUCEF_WL_XDG_WM_BASE_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_xdg_wm_base_add_listener( const SWaylandApi* api, struct xdg_wm_base* proxy, const struct gucef_xdg_wm_base_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_xdg_wm_base_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "xdg_wm_base" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_xdg_wm_base_destroy( const SWaylandApi* api, struct xdg_wm_base* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct xdg_positioner*
gucef_xdg_wm_base_create_positioner( const SWaylandApi* api, struct xdg_wm_base* proxy )
{
    return (struct xdg_positioner*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_xdg_positioner_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline struct xdg_surface*
gucef_xdg_wm_base_get_xdg_surface( const SWaylandApi* api, struct xdg_wm_base* proxy, struct wl_surface* surface )
{
    return (struct xdg_surface*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, &gucef_xdg_surface_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, surface );
}
static inline void
gucef_xdg_wm_base_pong( const SWaylandApi* api, struct xdg_wm_base* proxy, uint32_t serial )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial );
}

#define GUCEF_WL_XDG_POSITIONER_VERSION 7
#define GUCEF_WL_XDG_POSITIONER_ERROR_INVALID_INPUT 0
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_NONE 0
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_TOP 1
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_BOTTOM 2
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_LEFT 3
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_RIGHT 4
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_TOP_LEFT 5
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_BOTTOM_LEFT 6
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_TOP_RIGHT 7
#define GUCEF_WL_XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT 8
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_NONE 0
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_TOP 1
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_BOTTOM 2
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_LEFT 3
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_RIGHT 4
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_TOP_LEFT 5
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_BOTTOM_LEFT 6
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_TOP_RIGHT 7
#define GUCEF_WL_XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT 8
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE 0
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X 1
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y 2
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X 4
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y 8
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X 16
#define GUCEF_WL_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y 32
#define GUCEF_WL_XDG_POSITIONER_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_POSITIONER_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_SIZE_OPCODE 1
#define GUCEF_WL_XDG_POSITIONER_SET_SIZE_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_ANCHOR_RECT_OPCODE 2
#define GUCEF_WL_XDG_POSITIONER_SET_ANCHOR_RECT_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_ANCHOR_OPCODE 3
#define GUCEF_WL_XDG_POSITIONER_SET_ANCHOR_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_GRAVITY_OPCODE 4
#define GUCEF_WL_XDG_POSITIONER_SET_GRAVITY_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT_OPCODE 5
#define GUCEF_WL_XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_OFFSET_OPCODE 6
#define GUCEF_WL_XDG_POSITIONER_SET_OFFSET_SINCE_VERSION 1
#define GUCEF_WL_XDG_POSITIONER_SET_REACTIVE_OPCODE 7
#define GUCEF_WL_XDG_POSITIONER_SET_REACTIVE_SINCE_VERSION 3
#define GUCEF_WL_XDG_POSITIONER_SET_PARENT_SIZE_OPCODE 8
#define GUCEF_WL_XDG_POSITIONER_SET_PARENT_SIZE_SINCE_VERSION 3
#define GUCEF_WL_XDG_POSITIONER_SET_PARENT_CONFIGURE_OPCODE 9
#define GUCEF_WL_XDG_POSITIONER_SET_PARENT_CONFIGURE_SINCE_VERSION 3
static inline void
gucef_xdg_positioner_destroy( const SWaylandApi* api, struct xdg_positioner* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_xdg_positioner_set_size( const SWaylandApi* api, struct xdg_positioner* proxy, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, width, height );
}
static inline void
gucef_xdg_positioner_set_anchor_rect( const SWaylandApi* api, struct xdg_positioner* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline void
gucef_xdg_positioner_set_anchor( const SWaylandApi* api, struct xdg_positioner* proxy, uint32_t anchor )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, anchor );
}
static inline void
gucef_xdg_positioner_set_gravity( const SWaylandApi* api, struct xdg_positioner* proxy, uint32_t gravity )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 4, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, gravity );
}
static inline void
gucef_xdg_positioner_set_constraint_adjustment( const SWaylandApi* api, struct xdg_positioner* proxy, uint32_t constraint_adjustment )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 5, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, constraint_adjustment );
}
static inline void
gucef_xdg_positioner_set_offset( const SWaylandApi* api, struct xdg_positioner* proxy, int32_t x, int32_t y )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 6, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y );
}
static inline void
gucef_xdg_positioner_set_reactive( const SWaylandApi* api, struct xdg_positioner* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 7, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_xdg_positioner_set_parent_size( const SWaylandApi* api, struct xdg_positioner* proxy, int32_t parent_width, int32_t parent_height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 8, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, parent_width, parent_height );
}
static inline void
gucef_xdg_positioner_set_parent_configure( const SWaylandApi* api, struct xdg_positioner* proxy, uint32_t serial )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 9, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial );
}

#define GUCEF_WL_XDG_SURFACE_VERSION 7
#define GUCEF_WL_XDG_SURFACE_ERROR_NOT_CONSTRUCTED 1
#define GUCEF_WL_XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED 2
#define GUCEF_WL_XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER 3
#define GUCEF_WL_XDG_SURFACE_ERROR_INVALID_SERIAL 4
#define GUCEF_WL_XDG_SURFACE_ERROR_INVALID_SIZE 5
#define GUCEF_WL_XDG_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT 6
#define GUCEF_WL_XDG_SURFACE_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_SURFACE_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_SURFACE_GET_TOPLEVEL_OPCODE 1
#define GUCEF_WL_XDG_SURFACE_GET_TOPLEVEL_SINCE_VERSION 1
#define GUCEF_WL_XDG_SURFACE_GET_POPUP_OPCODE 2
#define GUCEF_WL_XDG_SURFACE_GET_POPUP_SINCE_VERSION 1
#define GUCEF_WL_XDG_SURFACE_SET_WINDOW_GEOMETRY_OPCODE 3
#define GUCEF_WL_XDG_SURFACE_SET_WINDOW_GEOMETRY_SINCE_VERSION 1
#define GUCEF_WL_XDG_SURFACE_ACK_CONFIGURE_OPCODE 4
#define GUCEF_WL_XDG_SURFACE_ACK_CONFIGURE_SINCE_VERSION 1
#define GUCEF_WL_XDG_SURFACE_CONFIGURE_EVENT_OPCODE 0
#define GUCEF_WL_XDG_SURFACE_CONFIGURE_EVENT_SINCE_VERSION 1
struct gucef_xdg_surface_listener {
    void (*configure)( void* data, struct xdg_surface* xdg_surface, uint32_t serial );
};
#define GUCEF_WL_XDG_SURFACE_EVENT_COUNT 1
typedef char gucef_xdg_surface_listener_has_a_slot_per_event[ sizeof( struct gucef_xdg_surface_listener ) == GUCEF_WL_XDG_SURFACE_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_xdg_surface_add_listener( const SWaylandApi* api, struct xdg_surface* proxy, const struct gucef_xdg_surface_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_xdg_surface_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "xdg_surface" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_xdg_surface_destroy( const SWaylandApi* api, struct xdg_surface* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct xdg_toplevel*
gucef_xdg_surface_get_toplevel( const SWaylandApi* api, struct xdg_surface* proxy )
{
    return (struct xdg_toplevel*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_xdg_toplevel_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline struct xdg_popup*
gucef_xdg_surface_get_popup( const SWaylandApi* api, struct xdg_surface* proxy, struct xdg_surface* parent, struct xdg_positioner* positioner )
{
    return (struct xdg_popup*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, &gucef_xdg_popup_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, parent, positioner );
}
static inline void
gucef_xdg_surface_set_window_geometry( const SWaylandApi* api, struct xdg_surface* proxy, int32_t x, int32_t y, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline void
gucef_xdg_surface_ack_configure( const SWaylandApi* api, struct xdg_surface* proxy, uint32_t serial )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 4, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial );
}

#define GUCEF_WL_XDG_TOPLEVEL_VERSION 7
#define GUCEF_WL_XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE 0
#define GUCEF_WL_XDG_TOPLEVEL_ERROR_INVALID_PARENT 1
#define GUCEF_WL_XDG_TOPLEVEL_ERROR_INVALID_SIZE 2
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_NONE 0
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_TOP 1
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM 2
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_LEFT 4
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT 5
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT 6
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_RIGHT 8
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT 9
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT 10
#define GUCEF_WL_XDG_TOPLEVEL_STATE_MAXIMIZED 1
#define GUCEF_WL_XDG_TOPLEVEL_STATE_FULLSCREEN 2
#define GUCEF_WL_XDG_TOPLEVEL_STATE_RESIZING 3
#define GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED 4
#define GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_LEFT 5
#define GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_RIGHT 6
#define GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_TOP 7
#define GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_BOTTOM 8
#define GUCEF_WL_XDG_TOPLEVEL_STATE_SUSPENDED 9
#define GUCEF_WL_XDG_TOPLEVEL_STATE_CONSTRAINED_LEFT 10
#define GUCEF_WL_XDG_TOPLEVEL_STATE_CONSTRAINED_RIGHT 11
#define GUCEF_WL_XDG_TOPLEVEL_STATE_CONSTRAINED_TOP 12
#define GUCEF_WL_XDG_TOPLEVEL_STATE_CONSTRAINED_BOTTOM 13
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU 1
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE 2
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN 3
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE 4
#define GUCEF_WL_XDG_TOPLEVEL_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_TOPLEVEL_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_PARENT_OPCODE 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_PARENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_TITLE_OPCODE 2
#define GUCEF_WL_XDG_TOPLEVEL_SET_TITLE_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_APP_ID_OPCODE 3
#define GUCEF_WL_XDG_TOPLEVEL_SET_APP_ID_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SHOW_WINDOW_MENU_OPCODE 4
#define GUCEF_WL_XDG_TOPLEVEL_SHOW_WINDOW_MENU_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_MOVE_OPCODE 5
#define GUCEF_WL_XDG_TOPLEVEL_MOVE_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_OPCODE 6
#define GUCEF_WL_XDG_TOPLEVEL_RESIZE_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_MAX_SIZE_OPCODE 7
#define GUCEF_WL_XDG_TOPLEVEL_SET_MAX_SIZE_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_MIN_SIZE_OPCODE 8
#define GUCEF_WL_XDG_TOPLEVEL_SET_MIN_SIZE_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_MAXIMIZED_OPCODE 9
#define GUCEF_WL_XDG_TOPLEVEL_SET_MAXIMIZED_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_UNSET_MAXIMIZED_OPCODE 10
#define GUCEF_WL_XDG_TOPLEVEL_UNSET_MAXIMIZED_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_FULLSCREEN_OPCODE 11
#define GUCEF_WL_XDG_TOPLEVEL_SET_FULLSCREEN_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_UNSET_FULLSCREEN_OPCODE 12
#define GUCEF_WL_XDG_TOPLEVEL_UNSET_FULLSCREEN_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_SET_MINIMIZED_OPCODE 13
#define GUCEF_WL_XDG_TOPLEVEL_SET_MINIMIZED_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_CONFIGURE_EVENT_OPCODE 0
#define GUCEF_WL_XDG_TOPLEVEL_CONFIGURE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_CLOSE_EVENT_OPCODE 1
#define GUCEF_WL_XDG_TOPLEVEL_CLOSE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_TOPLEVEL_CONFIGURE_BOUNDS_EVENT_OPCODE 2
#define GUCEF_WL_XDG_TOPLEVEL_CONFIGURE_BOUNDS_EVENT_SINCE_VERSION 4
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_EVENT_OPCODE 3
#define GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_EVENT_SINCE_VERSION 5
struct gucef_xdg_toplevel_listener {
    void (*configure)( void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states );
    void (*close)( void* data, struct xdg_toplevel* xdg_toplevel );
    void (*configure_bounds)( void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height );
    void (*wm_capabilities)( void* data, struct xdg_toplevel* xdg_toplevel, struct wl_array* capabilities );
};
#define GUCEF_WL_XDG_TOPLEVEL_EVENT_COUNT 4
typedef char gucef_xdg_toplevel_listener_has_a_slot_per_event[ sizeof( struct gucef_xdg_toplevel_listener ) == GUCEF_WL_XDG_TOPLEVEL_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_xdg_toplevel_add_listener( const SWaylandApi* api, struct xdg_toplevel* proxy, const struct gucef_xdg_toplevel_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_xdg_toplevel_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "xdg_toplevel" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_xdg_toplevel_destroy( const SWaylandApi* api, struct xdg_toplevel* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_xdg_toplevel_set_parent( const SWaylandApi* api, struct xdg_toplevel* proxy, struct xdg_toplevel* parent )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, parent );
}
static inline void
gucef_xdg_toplevel_set_title( const SWaylandApi* api, struct xdg_toplevel* proxy, const char* title )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, title );
}
static inline void
gucef_xdg_toplevel_set_app_id( const SWaylandApi* api, struct xdg_toplevel* proxy, const char* app_id )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, app_id );
}
static inline void
gucef_xdg_toplevel_show_window_menu( const SWaylandApi* api, struct xdg_toplevel* proxy, struct wl_seat* seat, uint32_t serial, int32_t x, int32_t y )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 4, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, seat, serial, x, y );
}
static inline void
gucef_xdg_toplevel_move( const SWaylandApi* api, struct xdg_toplevel* proxy, struct wl_seat* seat, uint32_t serial )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 5, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, seat, serial );
}
static inline void
gucef_xdg_toplevel_resize( const SWaylandApi* api, struct xdg_toplevel* proxy, struct wl_seat* seat, uint32_t serial, uint32_t edges )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 6, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, seat, serial, edges );
}
static inline void
gucef_xdg_toplevel_set_max_size( const SWaylandApi* api, struct xdg_toplevel* proxy, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 7, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, width, height );
}
static inline void
gucef_xdg_toplevel_set_min_size( const SWaylandApi* api, struct xdg_toplevel* proxy, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 8, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, width, height );
}
static inline void
gucef_xdg_toplevel_set_maximized( const SWaylandApi* api, struct xdg_toplevel* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 9, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_xdg_toplevel_unset_maximized( const SWaylandApi* api, struct xdg_toplevel* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 10, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_xdg_toplevel_set_fullscreen( const SWaylandApi* api, struct xdg_toplevel* proxy, struct wl_output* output )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 11, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, output );
}
static inline void
gucef_xdg_toplevel_unset_fullscreen( const SWaylandApi* api, struct xdg_toplevel* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 12, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_xdg_toplevel_set_minimized( const SWaylandApi* api, struct xdg_toplevel* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 13, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}

#define GUCEF_WL_XDG_POPUP_VERSION 7
#define GUCEF_WL_XDG_POPUP_ERROR_INVALID_GRAB 0
#define GUCEF_WL_XDG_POPUP_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_POPUP_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_POPUP_GRAB_OPCODE 1
#define GUCEF_WL_XDG_POPUP_GRAB_SINCE_VERSION 1
#define GUCEF_WL_XDG_POPUP_REPOSITION_OPCODE 2
#define GUCEF_WL_XDG_POPUP_REPOSITION_SINCE_VERSION 3
#define GUCEF_WL_XDG_POPUP_CONFIGURE_EVENT_OPCODE 0
#define GUCEF_WL_XDG_POPUP_CONFIGURE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_POPUP_POPUP_DONE_EVENT_OPCODE 1
#define GUCEF_WL_XDG_POPUP_POPUP_DONE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_XDG_POPUP_REPOSITIONED_EVENT_OPCODE 2
#define GUCEF_WL_XDG_POPUP_REPOSITIONED_EVENT_SINCE_VERSION 3
struct gucef_xdg_popup_listener {
    void (*configure)( void* data, struct xdg_popup* xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height );
    void (*popup_done)( void* data, struct xdg_popup* xdg_popup );
    void (*repositioned)( void* data, struct xdg_popup* xdg_popup, uint32_t token );
};
#define GUCEF_WL_XDG_POPUP_EVENT_COUNT 3
typedef char gucef_xdg_popup_listener_has_a_slot_per_event[ sizeof( struct gucef_xdg_popup_listener ) == GUCEF_WL_XDG_POPUP_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_xdg_popup_add_listener( const SWaylandApi* api, struct xdg_popup* proxy, const struct gucef_xdg_popup_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_xdg_popup_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "xdg_popup" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_xdg_popup_destroy( const SWaylandApi* api, struct xdg_popup* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_xdg_popup_grab( const SWaylandApi* api, struct xdg_popup* proxy, struct wl_seat* seat, uint32_t serial )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, seat, serial );
}
static inline void
gucef_xdg_popup_reposition( const SWaylandApi* api, struct xdg_popup* proxy, struct xdg_positioner* positioner, uint32_t token )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, positioner, token );
}

/* ---- viewporter ---- */

#define GUCEF_WL_WP_VIEWPORTER_VERSION 1
#define GUCEF_WL_WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS 0
#define GUCEF_WL_WP_VIEWPORTER_DESTROY_OPCODE 0
#define GUCEF_WL_WP_VIEWPORTER_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_VIEWPORTER_GET_VIEWPORT_OPCODE 1
#define GUCEF_WL_WP_VIEWPORTER_GET_VIEWPORT_SINCE_VERSION 1
static inline void
gucef_wp_viewporter_destroy( const SWaylandApi* api, struct wp_viewporter* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct wp_viewport*
gucef_wp_viewporter_get_viewport( const SWaylandApi* api, struct wp_viewporter* proxy, struct wl_surface* surface )
{
    return (struct wp_viewport*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_wp_viewport_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, surface );
}

#define GUCEF_WL_WP_VIEWPORT_VERSION 1
#define GUCEF_WL_WP_VIEWPORT_ERROR_BAD_VALUE 0
#define GUCEF_WL_WP_VIEWPORT_ERROR_BAD_SIZE 1
#define GUCEF_WL_WP_VIEWPORT_ERROR_OUT_OF_BUFFER 2
#define GUCEF_WL_WP_VIEWPORT_ERROR_NO_SURFACE 3
#define GUCEF_WL_WP_VIEWPORT_DESTROY_OPCODE 0
#define GUCEF_WL_WP_VIEWPORT_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_VIEWPORT_SET_SOURCE_OPCODE 1
#define GUCEF_WL_WP_VIEWPORT_SET_SOURCE_SINCE_VERSION 1
#define GUCEF_WL_WP_VIEWPORT_SET_DESTINATION_OPCODE 2
#define GUCEF_WL_WP_VIEWPORT_SET_DESTINATION_SINCE_VERSION 1
static inline void
gucef_wp_viewport_destroy( const SWaylandApi* api, struct wp_viewport* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_wp_viewport_set_source( const SWaylandApi* api, struct wp_viewport* proxy, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, x, y, width, height );
}
static inline void
gucef_wp_viewport_set_destination( const SWaylandApi* api, struct wp_viewport* proxy, int32_t width, int32_t height )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, width, height );
}

/* ---- fractional_scale_v1 ---- */

#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_VERSION 1
#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS 0
#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_DESTROY_OPCODE 0
#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_GET_FRACTIONAL_SCALE_OPCODE 1
#define GUCEF_WL_WP_FRACTIONAL_SCALE_MANAGER_V1_GET_FRACTIONAL_SCALE_SINCE_VERSION 1
static inline void
gucef_wp_fractional_scale_manager_v1_destroy( const SWaylandApi* api, struct wp_fractional_scale_manager_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct wp_fractional_scale_v1*
gucef_wp_fractional_scale_manager_v1_get_fractional_scale( const SWaylandApi* api, struct wp_fractional_scale_manager_v1* proxy, struct wl_surface* surface )
{
    return (struct wp_fractional_scale_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_wp_fractional_scale_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, surface );
}

#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_VERSION 1
#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_DESTROY_OPCODE 0
#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_PREFERRED_SCALE_EVENT_OPCODE 0
#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_PREFERRED_SCALE_EVENT_SINCE_VERSION 1
struct gucef_wp_fractional_scale_v1_listener {
    void (*preferred_scale)( void* data, struct wp_fractional_scale_v1* wp_fractional_scale_v1, uint32_t scale );
};
#define GUCEF_WL_WP_FRACTIONAL_SCALE_V1_EVENT_COUNT 1
typedef char gucef_wp_fractional_scale_v1_listener_has_a_slot_per_event[ sizeof( struct gucef_wp_fractional_scale_v1_listener ) == GUCEF_WL_WP_FRACTIONAL_SCALE_V1_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_wp_fractional_scale_v1_add_listener( const SWaylandApi* api, struct wp_fractional_scale_v1* proxy, const struct gucef_wp_fractional_scale_v1_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_wp_fractional_scale_v1_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "wp_fractional_scale_v1" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_wp_fractional_scale_v1_destroy( const SWaylandApi* api, struct wp_fractional_scale_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

/* ---- xdg_decoration_unstable_v1 ---- */

#define GUCEF_WL_ZXDG_DECORATION_MANAGER_V1_VERSION 2
#define GUCEF_WL_ZXDG_DECORATION_MANAGER_V1_DESTROY_OPCODE 0
#define GUCEF_WL_ZXDG_DECORATION_MANAGER_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_DECORATION_MANAGER_V1_GET_TOPLEVEL_DECORATION_OPCODE 1
#define GUCEF_WL_ZXDG_DECORATION_MANAGER_V1_GET_TOPLEVEL_DECORATION_SINCE_VERSION 1
static inline void
gucef_zxdg_decoration_manager_v1_destroy( const SWaylandApi* api, struct zxdg_decoration_manager_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct zxdg_toplevel_decoration_v1*
gucef_zxdg_decoration_manager_v1_get_toplevel_decoration( const SWaylandApi* api, struct zxdg_decoration_manager_v1* proxy, struct xdg_toplevel* toplevel )
{
    return (struct zxdg_toplevel_decoration_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_zxdg_toplevel_decoration_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, toplevel );
}

#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_VERSION 2
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_ERROR_UNCONFIGURED_BUFFER 0
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED 2
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_ERROR_INVALID_MODE 3
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE 2
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_DESTROY_OPCODE 0
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_SET_MODE_OPCODE 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_SET_MODE_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_UNSET_MODE_OPCODE 2
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_UNSET_MODE_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_CONFIGURE_EVENT_OPCODE 0
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_CONFIGURE_EVENT_SINCE_VERSION 1
struct gucef_zxdg_toplevel_decoration_v1_listener {
    void (*configure)( void* data, struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1, uint32_t mode );
};
#define GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_EVENT_COUNT 1
typedef char gucef_zxdg_toplevel_decoration_v1_listener_has_a_slot_per_event[ sizeof( struct gucef_zxdg_toplevel_decoration_v1_listener ) == GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_zxdg_toplevel_decoration_v1_add_listener( const SWaylandApi* api, struct zxdg_toplevel_decoration_v1* proxy, const struct gucef_zxdg_toplevel_decoration_v1_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_zxdg_toplevel_decoration_v1_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "zxdg_toplevel_decoration_v1" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_zxdg_toplevel_decoration_v1_destroy( const SWaylandApi* api, struct zxdg_toplevel_decoration_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_zxdg_toplevel_decoration_v1_set_mode( const SWaylandApi* api, struct zxdg_toplevel_decoration_v1* proxy, uint32_t mode )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, mode );
}
static inline void
gucef_zxdg_toplevel_decoration_v1_unset_mode( const SWaylandApi* api, struct zxdg_toplevel_decoration_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}

/* ---- cursor_shape_v1 ---- */

#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_VERSION 2
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_DESTROY_OPCODE 0
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_GET_POINTER_OPCODE 1
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_GET_POINTER_SINCE_VERSION 1
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_GET_TABLET_TOOL_V2_OPCODE 2
#define GUCEF_WL_WP_CURSOR_SHAPE_MANAGER_V1_GET_TABLET_TOOL_V2_SINCE_VERSION 1
static inline void
gucef_wp_cursor_shape_manager_v1_destroy( const SWaylandApi* api, struct wp_cursor_shape_manager_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct wp_cursor_shape_device_v1*
gucef_wp_cursor_shape_manager_v1_get_pointer( const SWaylandApi* api, struct wp_cursor_shape_manager_v1* proxy, struct wl_pointer* pointer )
{
    return (struct wp_cursor_shape_device_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_wp_cursor_shape_device_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, pointer );
}
static inline struct wp_cursor_shape_device_v1*
gucef_wp_cursor_shape_manager_v1_get_tablet_tool_v2( const SWaylandApi* api, struct wp_cursor_shape_manager_v1* proxy, struct zwp_tablet_tool_v2* tablet_tool )
{
    return (struct wp_cursor_shape_device_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, &gucef_wp_cursor_shape_device_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, tablet_tool );
}

#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_VERSION 2
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT 1
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CONTEXT_MENU 2
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP 3
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER 4
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS 5
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT 6
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CELL 7
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR 8
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT 9
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_VERTICAL_TEXT 10
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALIAS 11
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COPY 12
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE 13
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NO_DROP 14
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED 15
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRAB 16
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRABBING 17
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE 18
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE 19
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE 20
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE 21
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE 22
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE 23
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE 24
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE 25
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE 26
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE 27
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE 28
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE 29
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COL_RESIZE 30
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ROW_RESIZE 31
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL 32
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_IN 33
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT 34
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DND_ASK 35
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_RESIZE 36
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_ERROR_INVALID_SHAPE 1
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_DESTROY_OPCODE 0
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SET_SHAPE_OPCODE 1
#define GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SET_SHAPE_SINCE_VERSION 1
static inline void
gucef_wp_cursor_shape_device_v1_destroy( const SWaylandApi* api, struct wp_cursor_shape_device_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline void
gucef_wp_cursor_shape_device_v1_set_shape( const SWaylandApi* api, struct wp_cursor_shape_device_v1* proxy, uint32_t serial, uint32_t shape )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial, shape );
}

/* ---- xdg_output_unstable_v1 ---- */

#define GUCEF_WL_ZXDG_OUTPUT_MANAGER_V1_VERSION 3
#define GUCEF_WL_ZXDG_OUTPUT_MANAGER_V1_DESTROY_OPCODE 0
#define GUCEF_WL_ZXDG_OUTPUT_MANAGER_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_OUTPUT_MANAGER_V1_GET_XDG_OUTPUT_OPCODE 1
#define GUCEF_WL_ZXDG_OUTPUT_MANAGER_V1_GET_XDG_OUTPUT_SINCE_VERSION 1
static inline void
gucef_zxdg_output_manager_v1_destroy( const SWaylandApi* api, struct zxdg_output_manager_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct zxdg_output_v1*
gucef_zxdg_output_manager_v1_get_xdg_output( const SWaylandApi* api, struct zxdg_output_manager_v1* proxy, struct wl_output* output )
{
    return (struct zxdg_output_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_zxdg_output_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL, output );
}

#define GUCEF_WL_ZXDG_OUTPUT_V1_VERSION 3
#define GUCEF_WL_ZXDG_OUTPUT_V1_DESTROY_OPCODE 0
#define GUCEF_WL_ZXDG_OUTPUT_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_OUTPUT_V1_LOGICAL_POSITION_EVENT_OPCODE 0
#define GUCEF_WL_ZXDG_OUTPUT_V1_LOGICAL_POSITION_EVENT_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_OUTPUT_V1_LOGICAL_SIZE_EVENT_OPCODE 1
#define GUCEF_WL_ZXDG_OUTPUT_V1_LOGICAL_SIZE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_OUTPUT_V1_DONE_EVENT_OPCODE 2
#define GUCEF_WL_ZXDG_OUTPUT_V1_DONE_EVENT_SINCE_VERSION 1
#define GUCEF_WL_ZXDG_OUTPUT_V1_NAME_EVENT_OPCODE 3
#define GUCEF_WL_ZXDG_OUTPUT_V1_NAME_EVENT_SINCE_VERSION 2
#define GUCEF_WL_ZXDG_OUTPUT_V1_DESCRIPTION_EVENT_OPCODE 4
#define GUCEF_WL_ZXDG_OUTPUT_V1_DESCRIPTION_EVENT_SINCE_VERSION 2
struct gucef_zxdg_output_v1_listener {
    void (*logical_position)( void* data, struct zxdg_output_v1* zxdg_output_v1, int32_t x, int32_t y );
    void (*logical_size)( void* data, struct zxdg_output_v1* zxdg_output_v1, int32_t width, int32_t height );
    void (*done)( void* data, struct zxdg_output_v1* zxdg_output_v1 );
    void (*name)( void* data, struct zxdg_output_v1* zxdg_output_v1, const char* name );
    void (*description)( void* data, struct zxdg_output_v1* zxdg_output_v1, const char* description );
};
#define GUCEF_WL_ZXDG_OUTPUT_V1_EVENT_COUNT 5
typedef char gucef_zxdg_output_v1_listener_has_a_slot_per_event[ sizeof( struct gucef_zxdg_output_v1_listener ) == GUCEF_WL_ZXDG_OUTPUT_V1_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_zxdg_output_v1_add_listener( const SWaylandApi* api, struct zxdg_output_v1* proxy, const struct gucef_zxdg_output_v1_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_zxdg_output_v1_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "zxdg_output_v1" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_zxdg_output_v1_destroy( const SWaylandApi* api, struct zxdg_output_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

/* ---- xdg_activation_v1 ---- */

#define GUCEF_WL_XDG_ACTIVATION_V1_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_V1_DESTROY_OPCODE 0
#define GUCEF_WL_XDG_ACTIVATION_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_V1_GET_ACTIVATION_TOKEN_OPCODE 1
#define GUCEF_WL_XDG_ACTIVATION_V1_GET_ACTIVATION_TOKEN_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_V1_ACTIVATE_OPCODE 2
#define GUCEF_WL_XDG_ACTIVATION_V1_ACTIVATE_SINCE_VERSION 1
static inline void
gucef_xdg_activation_v1_destroy( const SWaylandApi* api, struct xdg_activation_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}
static inline struct xdg_activation_token_v1*
gucef_xdg_activation_v1_get_activation_token( const SWaylandApi* api, struct xdg_activation_v1* proxy )
{
    return (struct xdg_activation_token_v1*) api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, &gucef_xdg_activation_token_v1_interface, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, NULL );
}
static inline void
gucef_xdg_activation_v1_activate( const SWaylandApi* api, struct xdg_activation_v1* proxy, const char* token, struct wl_surface* surface )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, token, surface );
}

#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED 0
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_SERIAL_OPCODE 0
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_SERIAL_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_APP_ID_OPCODE 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_APP_ID_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_SURFACE_OPCODE 2
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_SURFACE_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_COMMIT_OPCODE 3
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_COMMIT_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DESTROY_OPCODE 4
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DESTROY_SINCE_VERSION 1
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DONE_EVENT_OPCODE 0
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DONE_EVENT_SINCE_VERSION 1
struct gucef_xdg_activation_token_v1_listener {
    void (*done)( void* data, struct xdg_activation_token_v1* xdg_activation_token_v1, const char* token );
};
#define GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_EVENT_COUNT 1
typedef char gucef_xdg_activation_token_v1_listener_has_a_slot_per_event[ sizeof( struct gucef_xdg_activation_token_v1_listener ) == GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_EVENT_COUNT * sizeof( void* ) ? 1 : -1 ];
static inline int
gucef_xdg_activation_token_v1_add_listener( const SWaylandApi* api, struct xdg_activation_token_v1* proxy, const struct gucef_xdg_activation_token_v1_listener* listener, void* data )
{
    /* libwayland aborts the process on an event whose entry in the listener is NULL: a listener that lacks one is refused */
    if ( !gucef_wl_listener_is_complete( &gucef_xdg_activation_token_v1_interface, (unsigned int) ( sizeof( *listener ) / sizeof( void* ) ), listener, api->proxy_get_version( (struct wl_proxy*) proxy ) ) )
    {
        if ( NULL != api->listener_rejected )
            api->listener_rejected( "xdg_activation_token_v1" );
        return -1;
    }
    return api->proxy_add_listener( (struct wl_proxy*) proxy, (void (**)(void)) listener, data );
}
static inline void
gucef_xdg_activation_token_v1_set_serial( const SWaylandApi* api, struct xdg_activation_token_v1* proxy, uint32_t serial, struct wl_seat* seat )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 0, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, serial, seat );
}
static inline void
gucef_xdg_activation_token_v1_set_app_id( const SWaylandApi* api, struct xdg_activation_token_v1* proxy, const char* app_id )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 1, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, app_id );
}
static inline void
gucef_xdg_activation_token_v1_set_surface( const SWaylandApi* api, struct xdg_activation_token_v1* proxy, struct wl_surface* surface )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 2, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0, surface );
}
static inline void
gucef_xdg_activation_token_v1_commit( const SWaylandApi* api, struct xdg_activation_token_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 3, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), 0 );
}
static inline void
gucef_xdg_activation_token_v1_destroy( const SWaylandApi* api, struct xdg_activation_token_v1* proxy )
{
    api->proxy_marshal_flags( (struct wl_proxy*) proxy, 4, NULL, api->proxy_get_version( (struct wl_proxy*) proxy ), GUCEF_WL_MARSHAL_FLAG_DESTROY );
}

#ifdef __cplusplus
}
#endif

#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */
